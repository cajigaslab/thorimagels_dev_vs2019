#ifndef NATIVE_MATLAB_ENGINE
#define NATIVE_MATLAB_ENGINE

#define DllExport extern "C" long __declspec(dllexport)

/// <summary>
/// Start matlab process.
/// </summary>
/// <param name="isSingle">true: will start new single engine; FALSE: singleton engine</param>
/// <param name="isFigureViable">true: enable show popup matlab figures. false: hide all figures</param>
/// <returns>positive number: engineId of the started engine; negative number or zero: failed.</returns>
DllExport int OpenEngine(bool isSingle, bool isFigureViable);

/// <summary>
/// Run matlab script with input path.
/// </summary>
/// <param name="engineId">id of started engine</param>
/// <param name="scriptPath">the path of the macro</param>
/// <param name="inputPath">the input experiment data folder</param>
/// <param name="logPath">the log file path</param>
/// <returns>TRUE: success; FALSE: failed.</returns>
DllExport int RunScript(int engineId, const char * scriptPath, const char * inputPath, const char * logPath);

/// <summary>
/// Execute matlab statement.
/// </summary>
/// <param name="engineId">id of started engine</param>
/// <param name="string">the excute statement</param>
/// <returns>TRUE: success; FALSE: failed.</returns>
DllExport int ExcuteStatement(int engineId, const char * statement);

/// <summary>
/// Set output buffer.
/// </summary>
/// <param name="engineId">id of started engine</param>
/// <param name="outputBuffer">the output buffer</param>
/// <param name="bufferSize">the output buffer size</param>
/// <returns>TRUE: success; FALSE: failed.</returns>
DllExport int SetOutputBuffer(int engineId, char* outputBuffer, int bufferLen);

/// <summary>
/// Close down matlab server .
/// </summary>
/// <param name="engineId">id of started engine</param>
/// <returns>TRUE: success; FALSE: failed.</returns>
DllExport int CloseEngine(int engineId);

#endif  //NATIVE_MATLAB_ENGIN