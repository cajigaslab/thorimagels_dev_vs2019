namespace ThorSharedTypes
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading.Tasks;

    /// <summary>
    /// The FileName class is designed to simplify the generation and manipulation of file names while
    /// ensuring they remain valid. Users are given can access and change the extension, name, and detected
    /// trailing file numbering separately or together. The class can make itself unique against a passed in
    /// criteria, and automatically converts any invalid filename characters in the name to valid ones.
    /// </summary>
    public class FileName
    {
        #region Fields

        public static int NumDigits = (int)Constants.DEFAULT_FILE_FORMAT_DIGITS;

        private const string INDEX_SEPARATOR = "_";

        private uint maxIterationCount = (uint)Math.Pow(10, ((int)Constants.MAX_FILE_FORMAT_DIGITS + 1));
        private string _nameNumber;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Constructs a new FileName object
        /// </summary>
        /// <param name="name"> The file name </param>
        public FileName(string name)
        {
            GetDigitCounts();
            FullName = name;
            if (name.LastIndexOf('.') >= 0)
            {
                FileExtension = name.Substring(name.LastIndexOf('.'), name.Length - name.LastIndexOf('.'));
            }
            if (null != FileExtension)
            {
                NameWithoutNumber = (name.Contains(INDEX_SEPARATOR) && !string.IsNullOrEmpty(NameNumber)) ?
                        name.Substring(0, name.Length - FileExtension.Length - NumDigits - INDEX_SEPARATOR.Length) :
                        name.Substring(0, name.Length - FileExtension.Length);
            }
        }

        /// <summary>
        /// Constructs a new FileName object with provided index separator
        /// </summary>
        /// <param name="name"> The file name </param>
        public FileName(string name, char separator)
        {
            GetDigitCounts();
            NameWithoutNumber = (name.LastIndexOf(separator) > 0) ? name.Substring(0, name.LastIndexOf(separator)) : name;

            if (name.LastIndexOf('.') >= 0)
            {
                FileExtension = name.Substring(name.LastIndexOf('.'), name.Length - name.LastIndexOf('.'));
                if (name.LastIndexOf(separator) < name.LastIndexOf('.') - 1)
                {
                    NameNumber = (0 < name.LastIndexOf(separator)) ?
                        name.Substring(name.LastIndexOf(separator) + 1, name.Length - name.LastIndexOf('.')) : "000";
                }
            }
            else if ((0 < name.LastIndexOf(separator)) && (name.LastIndexOf(separator) < (name.Length - 1)))
            {
                NameNumber = name.Substring(name.LastIndexOf(separator) + 1, name.Length - name.LastIndexOf(separator) - 1);
            }
            else
            {
                NameNumber = "000";
            }
        }

        /// <summary>
        /// Constructs a new FileName that is an exact copy of another one
        /// </summary>
        /// <param name="copyFrom"> The FileName object to be a copy of </param>
        public FileName(FileName copyFrom)
        {
            GetDigitCounts();
            NameNumberInt = copyFrom.NameNumberInt;
            NameWithoutNumber = copyFrom.NameWithoutNumber;
            FileExtension = copyFrom.FileExtension;
        }

        #endregion Constructors

        #region Properties

        /// <summary>
        /// The extension of this file name
        /// </summary>
        public string FileExtension
        {
            get;
            set;
        }

        /// <summary>
        /// The full file name, including extension. Returns a guaranteed valid file name. Input cannot be null or empty
        /// </summary>
        public string FullName
        {
            get
            {
                string name = String.Join("", new String[] { NameWithoutExtension, FileExtension });
                return MakeValid(name);
            }
            set
            {
                if (String.IsNullOrEmpty(value))
                    throw new ArgumentException("Argument Filename Cannot Be Null Or Empty");

                NameWithoutExtension = Path.GetFileNameWithoutExtension(value);
            }
        }

        /// <summary>
        /// The minimum number of digits in the appended number
        /// </summary>
        public int MinDigitsInNumber
        {
            get;
            set;
        }

        /// <summary>
        /// The trailing number of this file name formated as a string
        /// </summary>
        public string NameNumber
        {
            get
            {
                return _nameNumber;
            }
            set
            {
                _nameNumber = value;
                MinDigitsInNumber = _nameNumber.Length;
            }
        }

        /// <summary>
        /// The trailing number of this file name represented as a integer
        /// </summary>
        public uint NameNumberInt
        {
            get
            {
                uint value;
                uint.TryParse(NameNumber, out value);
                return value;
            }
            set
            {
                NameNumber = value.ToString("D" + Math.Max(MinDigitsInNumber, NumDigits));
            }
        }

        /// <summary>
        /// The separating string between the name and the name number
        /// </summary>
        public string NameNumberSeparator
        {
            get
            {
                if (NameNumber.Length > 0)
                {
                    return INDEX_SEPARATOR;
                }
                else
                {
                    return "";
                }
            }
        }

        /// <summary>
        /// The name of this file without an extension
        /// </summary>
        public string NameWithoutExtension
        {
            get
            {
                return String.Join(NameNumberSeparator, new String[] { NameWithoutNumber, NameNumber });
            }
            set
            {
                NameNumber = GetNumberAtEndAsString(value);

                NameWithoutNumber = value.Substring(0, value.Length - NameNumber.Length - (NameNumber.Length > 0 ? 1 : 0));
            }
        }

        /// <summary>
        /// The name of this file without any trailing numbering
        /// </summary>
        public string NameWithoutNumber
        {
            get;
            set;
        }

        /// <summary>
        /// Is this file name a valid file name
        /// </summary>
        public bool Valid
        {
            get
            {
                if (NameWithoutNumber.Length > 0 && FullName.IndexOfAny(Path.GetInvalidFileNameChars()) < 0)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        #endregion Properties

        #region Methods

        public static int GetDigitCounts()
        {
            string str = string.Empty;
            int imgIndxDigiCnts = NumDigits = (int)Constants.DEFAULT_FILE_FORMAT_DIGITS;
            try
            {
                System.Xml.XmlDocument appSettings = new System.Xml.XmlDocument();
                string appSettingsFile = ResourceManagerCS.GetApplicationSettingsFileString();
                ResourceManagerCS.BorrowDocMutexCS(SettingsFileType.APPLICATION_SETTINGS);
                appSettings.Load(appSettingsFile);
                ResourceManagerCS.ReturnDocMutexCS(SettingsFileType.APPLICATION_SETTINGS);
                System.Xml.XmlNode node = appSettings.SelectSingleNode("/ApplicationSettings/ImageNameFormat");
                if (XmlManager.GetAttribute(node, appSettings, "indexDigitCounts", ref str) && (Int32.TryParse(str, out imgIndxDigiCnts)))
                {
                    NumDigits = Math.Max(Math.Min(imgIndxDigiCnts, (int)Constants.MAX_FILE_FORMAT_DIGITS), 1);
                }
            }
            catch (DllNotFoundException)
            {
            }
            catch (Exception ex)
            {
                ResourceManagerCS.ReturnDocMutexCS(SettingsFileType.APPLICATION_SETTINGS);
                ThorLogging.ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Error, 1, "ThorSharedTypes GetDigitCounts error: " + ex.Message);
            }
            return NumDigits;
        }

        /// <summary>
        /// Increments this name by placing a number at the end, or if a number is already
        /// present at the end incrementing the number. Ex. file1900.txt => file1901.txt
        /// </summary>
        public void Increment()
        {
            NameNumberInt = ++NameNumberInt;
        }

        /// <summary>
        /// Continues to call increment until test for unique returns true, or max iterations are reached
        /// </summary>
        /// <param name="unique"> Function that takes in this FileName option and returns true if it is unique </param>
        /// <param name="maxIterations"> The maximum number of attempts to make it unique </param>
        /// <returns> True if the name was successfully made unique, false otherwise </returns>
        public bool MakeUnique(Func<FileName, bool> unique)
        {
            uint iteration = 0;

            while (!unique(this) && !string.IsNullOrEmpty(this.NameWithoutNumber))
            {
                if (iteration >= maxIterationCount)
                {
                    return false;
                }
                ++iteration;

                Increment();
            }

            return true;
        }

        /// <summary>
        /// Makes this file or directory unique with respect to all files contained in the input path.
        /// </summary>
        /// <param name="path"> The directory to make unique within </param>
        /// <param name="maxIterations"> How many attempts to make the filename unique </param>
        /// <returns> True if the name was successfully made unique, false otherwise </returns>
        public bool MakeUnique(String pathToDirectory)
        {
            return MakeUnique((FileName n) =>
                                        {
                                            return !(Directory.Exists(pathToDirectory + "\\" + n.FullName) ||
                                                          File.Exists(pathToDirectory + "\\" + n.FullName));
                                        }
                                        );
        }

        /// <summary>
        /// Count the number of occurrences of one string in another
        /// </summary>
        /// <param name="content"> The string to search in </param>
        /// <param name="search"> The string to search for </param>
        /// <returns> The number of times 'search' is found in 'content' </returns>
        private int CountOccurrences(string content, string search)
        {
            int sizeWithSearchStrings = content.Length;
            int sizeWithoutSearchStrings = content.Replace(search, "").Length;
            int sizeDifference = sizeWithSearchStrings - sizeWithoutSearchStrings;

            int count = sizeDifference / search.Length;
            return count;
        }

        /// <summary>
        /// Get the number at the end of a string as a string
        /// </summary>
        /// <param name="input"> The string to parse for a trailing number </param>
        /// <returns> An empty string if no number found, otherwise a string representing the number </returns>
        private string GetNumberAtEndAsString(string input)
        {
            return Regex.Match(input, @"_\d+$").Value.Replace(INDEX_SEPARATOR, "");
        }

        /// <summary>
        /// Makes a file name valid by replacing invalid characters (':','?', etc) with the input character
        /// </summary>
        /// <param name="replaceWith"> Valid character to substitute in place of invalid characters. Defaults to '_' and can be empty </param>
        private string MakeValid(string name, char replaceWith = '_')
        {
            foreach (char c in System.IO.Path.GetInvalidFileNameChars())
            {
                name = name.Replace(c, replaceWith);
            }
            return name;
        }

        #endregion Methods
    }
}