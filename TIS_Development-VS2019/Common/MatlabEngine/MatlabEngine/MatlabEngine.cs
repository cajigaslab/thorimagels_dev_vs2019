namespace MatlabEngineWrapper
{
    using System;
    using System.Collections.Concurrent;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;

    using Microsoft.Win32;

    public class MatlabEngine
    {
        #region Fields

        private static readonly Lazy<MatlabEngine> lazy = 
            new Lazy<MatlabEngine>(() => new MatlabEngine());

        private ConcurrentDictionary<int, bool> _asynchronousEngines = new ConcurrentDictionary<int, bool>();
        private bool _isEngineAvalid;
        private bool _isMatlabAvalid;
        private string _matlabExePath;
        private string _matlabRuntimeRoot;
        private bool _shouldCloseEngine;
        private int _signalEngine;

        #endregion Fields

        #region Constructors

        private MatlabEngine()
        {
            _signalEngine = 0;
            InitMatlab();
            InitEngine();
        }

        #endregion Constructors

        #region Properties

        public static MatlabEngine Instance
        {
            get { return lazy.Value; }
        }

        #endregion Properties

        #region Methods

        public void InitEngine()
        {
            _shouldCloseEngine = false;
        }

        public bool IsEngineValid()
        {
            return _isEngineAvalid;
        }

        public bool IsMatlabValid()
        {
            return _isMatlabAvalid;
        }

        public bool RunScript(string scriptPath, string inputPath, string logPath, bool isAsynchronous, bool isEngineVisiable)
        {
            if (!_isEngineAvalid) { return false; }
            bool result = true;
            if (isAsynchronous)
            {
                Task.Factory.StartNew(() => StartAsynchronousEngineWithScript(scriptPath, inputPath, logPath, isEngineVisiable));
            }
            else
            {
                if (_signalEngine < 1)
                {
                    _signalEngine = NMatlabEngine.OpenEngine(false, isEngineVisiable);
                    // The output buffer is only for sychronous mode

                    if (_signalEngine < 1) return false;
                }

                result = NMatlabEngine.RunScript(_signalEngine, scriptPath, inputPath, logPath) == 1;
                if (result)
                {
                    // run script with error, so close the engine and remove the log file.
                    result = RemoveLogFile(logPath);
                    if (!result)
                    {
                        StopSignalEngine();
                        RemoveLogFile(logPath);
                    }
                }
            }
            return result;
        }

        public bool StartMatlabApp(string scriptPath, string parameterPath)
        {
            if (!_isMatlabAvalid || _matlabExePath == null) { return false; }
            bool result = false;
            if (File.Exists(scriptPath))
            {
                if (File.Exists(parameterPath))
                {
                    var path = Path.GetDirectoryName(scriptPath);
                    var function = Path.GetFileNameWithoutExtension(scriptPath);
                    // Create the command to be executed {output} {func} {path to experiment}
                    var excuteF = string.Format("{0} {1} {2} {3} {4};", "[imData imInfo] =", function, "(\'" + parameterPath + "\')", "", "");
                    // In order to run the function, you have to move to the folder of the script first.
                    var process = new Process()
                    {
                        StartInfo = new ProcessStartInfo()
                        {
                            FileName = _matlabExePath,
                            Arguments =
                               " -r \"cd \'" + path + "\'; " +
                               excuteF +
                               "\""
                        }
                    };
                    process.Start();
                }
                else
                {
                    Process.Start(_matlabExePath);
                }
                result = true;
            }
            else
            {
                MessageBox.Show("Unable to find Matlab script " + scriptPath.ToString());
            }
            return result;
        }

        public void StopEngine()
        {
            if (!_isEngineAvalid) return;

            StopSignalEngine();

            _shouldCloseEngine = true;

            var avalidEngines = _asynchronousEngines.Where(ae => ae.Value == true).Select(ae => ae.Key).ToList();

            foreach (var e in avalidEngines)
            {
                NMatlabEngine.CloseEngine(e);
                bool result;
                _asynchronousEngines.TryRemove(e, out result);
            }
        }

        public void StopSignalEngine()
        {
            if (_signalEngine > 0)
            {
                NMatlabEngine.CloseEngine(_signalEngine);

                _signalEngine = 0;
            }
        }

        private void InitMatlab()
        {
            try
            {
                var name = "PATH";
                var pathvar = Environment.GetEnvironmentVariable(name);

                //check installed outside runtime engine
                bool isExternalRuntime = false;
                var keyStr = "Software\\MathWorks\\MATLAB Runtime\\";
                using (var rKey = Registry.LocalMachine.OpenSubKey(keyStr))
                {
                    if (rKey != null)
                    {
                        // default only use newest version; the runtime path will auto set to eviriment path when installed.
                        var maxVersion = rKey.GetSubKeyNames().Max();
                        keyStr += maxVersion;
                        var versionKey = Registry.LocalMachine.OpenSubKey(keyStr);
                        _matlabRuntimeRoot = versionKey.GetValue("MATLABROOT").ToString();
                        if (_matlabRuntimeRoot != null)
                        {
                            var versionStr = "v" + maxVersion.Replace(".", "");
                            var enginePath = _matlabRuntimeRoot + "\\" + versionStr + "\\bin\\win64";
                            if (Directory.Exists(enginePath))
                            {
                                if (pathvar == null || !pathvar.Contains(enginePath))
                                {
                                    var value = enginePath + ";" + pathvar;
                                    Environment.SetEnvironmentVariable(name, value, EnvironmentVariableTarget.Process);
                                }
                                isExternalRuntime = true;
                            }
                        }
                    }
                }

                // check installed matlab
                keyStr = "Software\\MathWorks\\MATLAB\\";
                using (var key = Registry.LocalMachine.OpenSubKey(keyStr))
                {
                    if (key != null)
                    {
                        var maxVersion = key.GetSubKeyNames().Max();
                        if (maxVersion != null)
                        {
                            keyStr += maxVersion;
                            var versionKey = Registry.LocalMachine.OpenSubKey(keyStr);
                            var matlabRoot = versionKey.GetValue("MATLABROOT").ToString();
                            if (matlabRoot != null)
                            {
                                _matlabExePath = matlabRoot + "\\bin\\matlab.exe";
                                if (File.Exists(_matlabExePath))
                                {
                                    _isMatlabAvalid = true;
                                    if (isExternalRuntime)
                                    {
                                        _isEngineAvalid = true;
                                    }
                                    else
                                    {
                                        //check default matlab installed with runtime
                                        var runtimePath = matlabRoot + "\\runtime\\win64";
                                        if (Directory.Exists(runtimePath))
                                        {
                                            var enginePath = matlabRoot + "\\bin\\win64";
                                            if (pathvar == null || !pathvar.Contains(enginePath))
                                            {
                                                var value = enginePath + ";" + pathvar;
                                                if (!value.Contains(runtimePath))
                                                {
                                                    value = runtimePath + ";" + value;
                                                }
                                                Environment.SetEnvironmentVariable(name, value, EnvironmentVariableTarget.Process);
                                            }
                                            _isEngineAvalid = true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            catch (Exception)
            {
                //react appropriately
            }
        }

        private bool RemoveLogFile(string logPath)
        {
            try
            {
                if (File.Exists(logPath))
                {
                    File.Delete(logPath);
                }
            }
            catch (Exception)
            {
                return false;
            }
            return true;
        }

        private bool StartAsynchronousEngineWithScript(string scriptPath, string inputPath, string logPath, bool isEngineVisiable)
        {
            var engine = _asynchronousEngines.FirstOrDefault(ae => ae.Value == true).Key;
            if (engine < 1)
            {
                engine = NMatlabEngine.OpenEngine(true, isEngineVisiable);
                if (engine < 1) return false;
            }
            _asynchronousEngines[engine] = false;

            bool result = false;
            result = NMatlabEngine.RunScript(engine, scriptPath, inputPath, logPath) == 1;

            bool isLogFileDeleted = RemoveLogFile(logPath);
            // run script with error, so close the engine and remove the log file.
            if (!isLogFileDeleted)
            {
                NMatlabEngine.CloseEngine(engine);
                bool r;
                _asynchronousEngines.TryRemove(engine, out r);
                RemoveLogFile(logPath);
            }
            else
            {
                if (_shouldCloseEngine)
                {
                    // auto close the engine after call StopEngine
                    NMatlabEngine.CloseEngine(engine);
                    bool r;
                    _asynchronousEngines.TryRemove(engine, out r);
                }
                else
                {
                    _asynchronousEngines[engine] = true;
                }
            }
            return result;
        }

        #endregion Methods

        #region Other

        //const int BUFFER_LENGTH = 1024*4;
        //#region Properties
        //private IntPtr _signalEngineBuffer;
        //public string EngineBuffer { get { return Marshal.PtrToStringAnsi(_signalEngineBuffer); } }
        //#endregion

        #endregion Other
    }

    internal class NMatlabEngine
    {
        #region Fields

        private const string DllName = "NativeMatlabEngine.dll";

        #endregion Fields

        #region Constructors

        private NMatlabEngine()
        {
        }

        #endregion Constructors

        #region Methods

        [DllImport(DllName, EntryPoint = "CloseEngine", CallingConvention = CallingConvention.Cdecl)]
        public static extern int CloseEngine(int engineId);

        [DllImport(DllName, EntryPoint = "ExcuteStatement", CallingConvention = CallingConvention.Cdecl)]
        public static extern int ExcuteStatement(int engineId, string statement);

        [DllImport(DllName, EntryPoint = "OpenEngine", CallingConvention = CallingConvention.Cdecl)]
        public static extern int OpenEngine(bool isSingle, bool isFigureViable);

        [DllImport(DllName, EntryPoint = "RunScript", CallingConvention = CallingConvention.Cdecl)]
        public static extern int RunScript(int engineId, string scriptPath, string inputPath, string logPath);

        [DllImport(DllName, EntryPoint = "SetOutputBuffer", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SetOutputBuffer(int engineId, IntPtr outputBuffer, int bufferLen);

        #endregion Methods
    }
}