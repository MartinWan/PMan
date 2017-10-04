README
  PMan is a command line tool for managing processes.
  Developed for CSC 360 - Operating Systems

  Commands:
    bg <program> [arguments] -- starts process with arguments
    bglist -- lists processes started by PMan
    bgkill <pid> -- terminates process with pid
    bgstop <pid> -- stops process with pid
    bgstart <pid> -- resumes process with pid
    pstat <pid> -- lists process statistics from /proc/stat & /proc/status/

INSTALLATION
  Compile Code by typing 'make' in the command line
  Alternatively, you can type 'gcc list.c rsi.c -lreadline -lhistory -o PMan'

RUNNING
  After installation, type ./PMan to start the program.
  Enjoy!
 

