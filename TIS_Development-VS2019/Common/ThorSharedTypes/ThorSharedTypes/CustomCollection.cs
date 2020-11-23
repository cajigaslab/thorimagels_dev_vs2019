//Inspired by the work of chopikadze
//http://stackoverflow.com/users/776448/chopikadze
namespace ThorSharedTypes
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Collections.Specialized;
    using System.ComponentModel;
    using System.Linq;
    using System.Linq.Expressions;
    using System.Threading;
    using System.Windows.Threading;

    public class CustomCollection<T> : IList<T>, IList, INotifyCollectionChanged, INotifyPropertyChanged
    {
        #region Fields

        private readonly IList<T> collection = new List<T>();
        private readonly Dispatcher dispatcher;
        private readonly ReaderWriterLock sync = new ReaderWriterLock();
        private readonly List<T> _InternalList = new List<T>();

        private Func<T, bool> _Filter;
        private IEnumerable<T> _Source;
        [NonSerialized]
        private object _syncRoot;

        #endregion Fields

        #region Constructors

        public CustomCollection()
        {
            dispatcher = Dispatcher.CurrentDispatcher;
        }

        public CustomCollection(IEnumerable<T> createFrom)
            : this()
        {
            Source = createFrom;
        }

        #endregion Constructors

        #region Events

        public event NotifyCollectionChangedEventHandler CollectionChanged;

        public event PropertyChangedEventHandler ItemPropertyChanged;

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public int Count
        {
            get
            {
                sync.AcquireReaderLock(Timeout.Infinite);
                var result = collection.Count;
                sync.ReleaseReaderLock();
                return result;
            }
        }

        public Func<T, bool> Filter
        {
            get { return _Filter; }
            set
            {
                //ignore if values are equal
                if (value == _Filter) return;

                _Filter = value;

                ApplyFilter();

                RaisePropertyChanged(() => Filter);
            }
        }

        public bool IsFixedSize
        {
            get { return false; }
        }

        public bool IsReadOnly
        {
            get
            {
                return false;
            }
        }

        public bool IsSynchronized
        {
            get { return false; }
        }

        public IEnumerable<T> Source
        {
            get
            {
                return _Source;
            }
            set
            {
                //ignore if values are equal
                if (value == _Source) return;

                if (_Source is INotifyCollectionChanged)
                    (_Source as INotifyCollectionChanged).CollectionChanged -= Source_CollectionChanged;

                _Source = value;

                InitFrom(_Source);

                if (_Source is INotifyCollectionChanged)
                    (_Source as INotifyCollectionChanged).CollectionChanged += Source_CollectionChanged;

                RaisePropertyChanged(() => Source);
            }
        }

        public object SyncRoot
        {
            get
            {
                if (_syncRoot == null)
                {
                    var c = collection as ICollection;
                    if (c != null)
                        _syncRoot = c.SyncRoot;
                    else
                        Interlocked.CompareExchange<object>(ref _syncRoot, new object(), null);
                }
                return _syncRoot;
            }
        }

        object IList.this[int index]
        {
            get { return this[index]; }
            set { this[index] = (T)value; }
        }

        #endregion Properties

        #region Indexers

        public T this[int index]
        {
            get
            {
                sync.AcquireReaderLock(Timeout.Infinite);

                if (collection?.Count > index)
                {
                    var result = collection[index];
                    sync.ReleaseReaderLock();
                    return result;
                }
                else
                {
                    var val = default(T);
                    return val;
                }
            }        
            set
            {
                sync.AcquireWriterLock(Timeout.Infinite);

                if (collection.Count == 0 || collection.Count <= index)
                {
                    sync.ReleaseWriterLock();
                    return;
                }

                var oldItem = collection[index];

                DetachPropertyChanged(oldItem);
                AttachPropertyChanged(value);

                if (ShouldBeHere(value))
                {
                    collection[index] = value;

                    // Notify about collection's changes
                    RaisePropertyChanged("Item[]");
                    RaiseCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Replace, value, oldItem, index));
                }
                else
                {
                    //
                    // Remove current item from collection
                    //

                    // Detach from PropertyChanged event
                    DetachPropertyChanged(oldItem);

                    // Remove item from collection
                    collection.RemoveAt(index);

                    // Notify about collection's changes
                    RaisePropertyChanged(() => Count);
                    RaisePropertyChanged("Item[]");
                    RaiseCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Remove, oldItem, index));
                }

                sync.ReleaseWriterLock();
            }
        }

        #endregion Indexers

        #region Methods

        public void Add(T item)
        {
            if (Thread.CurrentThread == dispatcher.Thread)
                DoAdd(item);
            else
                dispatcher.BeginInvoke((Action)(() => DoAdd(item)));
        }

        public int Add(object item)
        {
            if (Thread.CurrentThread == dispatcher.Thread)
                return DoAdd((T)item);
            else
            {
                var op = dispatcher.BeginInvoke(new Func<T, int>(DoAdd), item);
                if (op == null || op.Result == null)
                    return -1;
                return (int)op.Result;
            }
        }

        public void ApplyFilter()
        {
            sync.AcquireWriterLock(Timeout.Infinite);

            foreach (var item in _InternalList)
            {
                ApplyFilter(item);
            }

            sync.ReleaseWriterLock();
        }

        public void Clear()
        {
            if (Thread.CurrentThread == dispatcher.Thread)
                DoClear();
            else
                dispatcher.BeginInvoke((Action)(DoClear));
        }

        public bool Clone(ref CustomCollection<T> obj)
        {
            sync.AcquireReaderLock(Timeout.Infinite);
            var result = CloneInternal(ref obj);
            sync.ReleaseReaderLock();
            return result;
        }

        public bool Contains(object value)
        {
            return Contains((T)value);
        }

        public bool Contains(T item)
        {
            sync.AcquireReaderLock(Timeout.Infinite);
            var result = ContainsInternal(item);
            sync.ReleaseReaderLock();
            return result;
        }

        public void CopyTo(T[] array, int arrayIndex)
        {
            sync.AcquireWriterLock(Timeout.Infinite);
            collection.CopyTo(array, arrayIndex);
            sync.ReleaseWriterLock();
        }

        public void CopyTo(Array array, int index)
        {
            sync.AcquireWriterLock(Timeout.Infinite);
            for (var i = 0; i < collection.Count; i++)
            {
                array.SetValue(collection[i], index + i);
            }
            sync.ReleaseWriterLock();
        }

        public IEnumerator<T> GetEnumerator()
        {
            return collection.GetEnumerator();
        }

        public int IndexOf(object value)
        {
            return IndexOf((T)value);
        }

        public int IndexOf(T item)
        {
            sync.AcquireReaderLock(Timeout.Infinite);
            var result = collection.IndexOf(item);
            sync.ReleaseReaderLock();
            return result;
        }

        public void Insert(int index, object value)
        {
            Insert(index, (T)value);
        }

        public void Insert(int index, T item)
        {
            if (Thread.CurrentThread == dispatcher.Thread)
                DoInsert(index, item);
            else
                dispatcher.BeginInvoke((Action)(() => DoInsert(index, item)));
        }

        public void Remove(object value)
        {
            Remove((T)value);
        }

        public bool Remove(T item)
        {
            if (Thread.CurrentThread == dispatcher.Thread)
                return DoRemove(item);
            else
            {
                var op = dispatcher.BeginInvoke(new Func<T, bool>(DoRemove), item);
                if (op == null || op.Result == null)
                    return false;
                return (bool)op.Result;
            }
        }

        public void RemoveAt(int index)
        {
            if (Thread.CurrentThread == dispatcher.Thread)
                DoRemoveAt(index);
            else
                dispatcher.BeginInvoke((Action)(() => DoRemoveAt(index)));
        }

        public void Sort()
        {
            if (Thread.CurrentThread == dispatcher.Thread)
                SortInternal();
            else
            {
                var op = dispatcher.BeginInvoke(new Func<bool>(SortInternal));
            }
        }

        public bool Swap(int idx1, int idx2)
        {
            if (Thread.CurrentThread == dispatcher.Thread)
                return DoSwap(idx1, idx2);
            else
            {
                var op = dispatcher.BeginInvoke(new Func<int, int, bool>(DoSwap), idx1, idx2);
                if (op == null || op.Result == null)
                    return false;
                return (bool)op.Result;
            }
        }

        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            return collection.GetEnumerator();
        }

        private bool ApplyFilter(T item)
        {
            var contains = ContainsInternal(item);
            var containsAfter = contains;

            if (ShouldBeHere(item))
            {
                if (!contains)
                {
                    DoAddInternal(item, false);
                    containsAfter = true;
                }
            }
            else
            {
                if (contains)
                {
                    DoRemoveInternal(item, false);
                    containsAfter = false;
                }
            }
            return containsAfter;
        }

        private void AttachPropertyChanged(T item)
        {
            _InternalList.Add(item);
            if (item is INotifyPropertyChanged)
            {
                (item as INotifyPropertyChanged).PropertyChanged += Item_PropertyChanged;
            }
        }

        private bool CloneInternal(ref CustomCollection<T> obj)
        {
            foreach (T item in collection)
            {
                obj.Add(item);
            }
            return true;
        }

        private bool ContainsInternal(T item)
        {
            return collection.Contains(item);
        }

        private void DetachPropertyChanged(T item)
        {
            _InternalList.Remove(item);
            if (item is INotifyPropertyChanged)
            {
                (item as INotifyPropertyChanged).PropertyChanged -= Item_PropertyChanged;
            }
        }

        private int DoAdd(T item)
        {
            sync.AcquireWriterLock(Timeout.Infinite);

            var index = DoAddInternal(item, true);

            sync.ReleaseWriterLock();

            return index;
        }

        private int DoAddInternal(T item, bool attachMonitorChanges)
        {
            // Attach to PropertyChanged event for monitoring future properties' changes
            if (attachMonitorChanges)
                AttachPropertyChanged(item);

            // Check if it should be here
            if (!ShouldBeHere(item))
                return -1;

            // Add item to collection
            var index = collection.Count;
            collection.Add(item);

            // Notify about collection's changes
            RaisePropertyChanged(() => Count);
            RaisePropertyChanged("Item[]");
            RaiseCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Add, item, index));

            return index;
        }

        private void DoClear()
        {
            sync.AcquireWriterLock(Timeout.Infinite);

            DoClearInternal();

            sync.ReleaseWriterLock();
        }

        private void DoClearInternal()
        {
            // Detach from PropertyChanged events
            foreach (var item in _InternalList.ToArray())
                DetachPropertyChanged(item);

            // Clear collection
            collection.Clear();

            // Notify about collection's changes
            RaisePropertyChanged(() => Count);
            RaisePropertyChanged("Item[]");
            RaiseCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Reset));
        }

        private void DoInsert(int index, T item)
        {
            sync.AcquireWriterLock(Timeout.Infinite);

            // Attach to PropertyChanged event for monitoring future properties' changes
            AttachPropertyChanged(item);

            // Check if it should be here
            if (!ShouldBeHere(item))
                return;

            // Insert item in collection
            collection.Insert(index, item);

            // Notify about collection's changes
            RaisePropertyChanged(() => Count);
            RaisePropertyChanged("Item[]");
            RaiseCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Add, item, index));

            sync.ReleaseWriterLock();
        }

        private bool DoRemove(T item)
        {
            sync.AcquireWriterLock(Timeout.Infinite);

            var result = DoRemoveInternal(item, true);

            sync.ReleaseWriterLock();

            return result;
        }

        private void DoRemoveAt(int index)
        {
            sync.AcquireWriterLock(Timeout.Infinite);

            if (collection.Count == 0 || collection.Count <= index)
            {
                sync.ReleaseWriterLock();
                return;
            }

            var item = collection[index];

            // Detach from PropertyChanged event
            DetachPropertyChanged(item);

            // Remove item from collection
            collection.RemoveAt(index);

            // Notify about collection's changes
            RaisePropertyChanged(() => Count);
            RaisePropertyChanged("Item[]");
            RaiseCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Remove, item, index));

            sync.ReleaseWriterLock();
        }

        private bool DoRemoveInternal(T item, bool detachMonitorChanges)
        {
            // Check if item is still at collection
            var index = collection.IndexOf(item);
            if (index == -1)
                return false;

            // Detach from PropertyChanged event
            if (detachMonitorChanges)
                DetachPropertyChanged(item);

            // Remove item from collection
            var result = collection.Remove(item);

            // Notify about collection's changes
            if (result)
            {
                RaisePropertyChanged(() => Count);
                RaisePropertyChanged("Item[]");
                RaiseCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Remove, item, index));
            }

            return result;
        }

        private bool DoSwap(int idx1, int idx2)
        {
            sync.AcquireWriterLock(Timeout.Infinite);

            var result = DoSwapInternal(idx1, idx2);

            sync.ReleaseWriterLock();

            return result;
        }

        private bool DoSwapInternal(int idx1, int idx2)
        {
            // Check if idx1 or idx2 in the range:
            if(collection.Count < idx1 || collection.Count < idx2)
                return false;

            T tmpItem = collection[idx1];
            collection[idx1] = collection[idx2];
            collection[idx2] = tmpItem;

            // Notify about collection's changes
            RaisePropertyChanged("Item[]");
            RaiseCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Move));

            return true;
        }

        private void InitFrom(IEnumerable<T> source)
        {
            if (source == null)
            {
                Clear();
                return;
            }

            sync.AcquireWriterLock(Timeout.Infinite);

            foreach (var item in source)
            {
                DoAddInternal(item, true);
            }

            sync.ReleaseWriterLock();
        }

        private void Item_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            sync.AcquireWriterLock(Timeout.Infinite);

            var item = (T)sender;

            var containsAfter = ApplyFilter(item);

            if (containsAfter)
                if (ItemPropertyChanged != null)
                    ItemPropertyChanged(sender, e);

            sync.ReleaseWriterLock();
        }

        private void RaiseCollectionChanged(NotifyCollectionChangedEventArgs e)
        {
            if (CollectionChanged != null)
                CollectionChanged(this, e);
        }

        private void RaisePropertyChanged<TSource>(Expression<Func<TSource>> propertyExpression)
        {
            var propertyName = ((MemberExpression)propertyExpression.Body).Member.Name;
            RaisePropertyChanged(propertyName);
        }

        private void RaisePropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
        }

        private bool ShouldBeHere(T item)
        {
            return (Filter == null) || Filter(item);
        }

        private bool SortInternal()
        {
            int[] indexs = Enumerable.Range(0, collection.Count).ToArray();
            T[] keys = collection.ToArray();
            Array.Sort(keys, indexs);
            for (int i = 0; i < collection.Count; i++)
            {
                collection[i] = keys[i];
            }
            return true;
        }

        void Source_CollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            sync.AcquireWriterLock(Timeout.Infinite);

            if (e.OldItems != null)
            {
                foreach (var oldItem in e.OldItems.OfType<T>())
                {
                    DoRemoveInternal(oldItem, true);
                }
            }

            if (e.NewItems != null)
            {
                foreach (var newItem in e.NewItems.OfType<T>())
                {
                    DoAddInternal(newItem, true);
                }
            }

            sync.ReleaseWriterLock();
        }

        #endregion Methods
    }
}