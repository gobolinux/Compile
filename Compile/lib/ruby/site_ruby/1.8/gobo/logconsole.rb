#!/usr/bin/ruby (require)

# This is a mixin for logging purposes.  Dependent on @logLevel
module GoboLogConsole
  LogLevelError=10
  LogLevelTerse=20
  LogLevelNormal=30
  LogLevelVerbose=40
  LogLevelDebug=50

  attr_reader :logLevel

  def setLogLevel(level)
    @logLevel = level
  end

  def logFunction(stream, message, color = "")
    stream.puts message
    stream.flush
  end

  def ask(message)
    $stdout << message
    $stdout.flush
    return $stdin.readline.chomp
  end # End of ask

  def askContinue(message = nil)
    return if @logLevel < LogLevelNormal
    puts message if message
    response = ask("Press Enter to continue or Ctrl-C to cancel.")
    stop if response.downcase == "n"
  end # End of askContinue

  def logError(message = "")
    logFunction($stderr, message) if @logLevel >= LogLevelError
  end
  def logTerse(message = "")
    logFunction($stdout, message) if @logLevel >= LogLevelTerse
  end
  def logNormal(message = "")
    logFunction($stdout, message) if @logLevel >= LogLevelNormal
  end
  def logVerbose(message = "")
    logFunction($stdout, message) if @logLevel >= LogLevelVerbose
  end
  def logDebug(message = "")
    logFunction($stderr, message) if @logLevel >= LogLevelDebug
  end


end # End of module GoboLogConsole
