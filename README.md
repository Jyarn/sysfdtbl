# Usage
`make` - run build task\
`make run` - run program with RARGS passed as command arguments\
`make gdb` - run program as above but in gdb\
`make vl`  - run program like in `make run` but through valgrind\
`make clean` - deletes all generated .o files, the executable and the compositeTable table if they exist\
see documentation for main (in Design/main) for accepted arguments\
\
Default behaviour is ./sysfdtl.out --composite\
`-std=gnuc99` was used because the compiler kept complaining about undefined references to stat

# Changes
I made a few aesthetic changes to the assignment:\
I swapped the Inode and Filename columns in the table, so the output looked cleaner (if the filename was too long it messed up the Inode columns)\
I also add a " >>> Targeting ..." in the output

# Files

`fileDesc.*`  -   fetch information to displayed by main
`IO.*`        -   print out to stream binary, or text mode
`misc.*`      -   miscellaneous functionions
`main.c`      -   command processing and main program logic
`test.sh`     -   simple script to benchmark for the bonus

# Design
### fileDesc.*
Information for each file descriptor processed is put into a fdDesc struct which contains its filename, its inode read by stat, and its file descriptor. An array of fdDesc structs are kept in a pidFdDesc, which groups all the file descriptors by their PID. It also keeps track of the number of file descriptors, its PID, and the next node in the list of pidFdDesc structs
File descriptors are processed something like this:
~~~
// If we want fetch all file descriptors for a user
for pid in /proc:
    if (getUid("/proc/[pid]/") == geteuid() ):
        for fd in "/proc/[pid]/fd"
            write into our CAD

return CAD

// If we just want to fetch all file descriptor for a specific pid
for fd in "/proc/[pid]/fd"
    write into our CAD

return CAD
~~~

`fetchAll` is the outer loop in the first sample and `frPidPathFtFdInfo` is the inner loop\
And `fetchSingle` calls `frPidPathFtFdInfo` and returns a piFdDesc\
`fetchStats` fetches the stats of a given file descriptor and writes it into bff using stat and readlink\
\
readlink is used to fetch the filename, like it said in the man proc documentation\
stat was used to fetch the inodes even though it doesn't match the output of `ls -li /proc/[pid]/fd/ -`. I assumed that `ls -li` used lstat rather than stat because it doesn't match the inode given by `readlink` (the inode in `type:[inode]`)

### IO.*

Wraps printf, fprintf, and fwrite into a variadic function\
A variadic function was used since it lets me specify a variable number of arguments, which was needed if I wanted to wrap encapsulate `printf`, `fprintf`, and `fwrite` together. My main reference for implementing this was https://www.youtube.com/watch?v=S-ak715zIIE.\
Around 3m:00s, he says that this is how all `printf` implementations implement `printf`. So I decided to call `vfprintf` to print in ascii mode\
Binary mode, calls `vsprintf` to process the string, and then calls `fwrite` to print\
What mode to print in is specified by the `printMode` enum

### main.c

Processes the command line arguments and is implements the main program logic. Accepted arguments are:
`--per-process`\
`--systemWide`\
`--Vnodes`\
`--composite`\
`--output_TXT`\
`--output_binary`\
`--threshold=x`, `--thresholdx`, `threshold x`, where x is the threshold\
and 1 positional argument assumed to be at the start (argv[1]). If argv[1] == "self" or atoi(argv[1]) <= 0 then the current PID is targeted instead. If argv[1] is not a number then all PIDs for the current user are targeted instead\
\
Tables are printed in the order they are presented in the command line arguments and duplicated are allowed, so:\
./sysfdtbl.out --systemWide --composite --systemWide, prints the systemWide, composite, and systemWide tables in that order\
What tables are printed and when is handled by a queue that stores everything in the order they were in when passed to the command line. If the queue is empty (qHead == 0), then this means no tables were specified so it just prints the composite table\
\
`printTable` and `printThresh`, print the table and threshold respectivley. `printTable` takes a series of flags (in a bitmask). Possible flags are: PRINT_INODES, PRINT_FLNAME, PRINT_FLDESC, PRINT_PROCID, PRINT_LNNUMS. `printTable` is implemented so that it is able to print many different table types. Not using a bitmask would make the argument list extremely long and hard to read, so this is why a bitmask was used. `printThresh` loops through the list of pidFdDesc, reads the size, and prints it if its size is strictly larger than threshold. Every 10 PIDs a newline is started


# Bonus

|  Trial no.|   Binary  |   ascii   |
| --------- | --------- | --------- |
|    1      |   0.023s  |   0.020s  |
|    2      |   0.023s  |   0.025s  |
|    3      |   0.022s  |   0.021s  |
|    4      |   0.022s  |   0.021s  |
|    5      |   0.021s  |   0.029s  |
|    6      |   0.022s  |   0.019s  |
|    7      |   0.021s  |   0.021s  |
|    8      |   0.028s  |   0.020s  |
|    9      |   0.022s  |   0.020s  |
|   10      |   0.023s  |   0.021s  |
|  Average  |  0.0227s  |  0.0217s  |

The benchmarks for binary, and ascii seem to be off by 0.001s. The similar times could because I use glibc (vsprintf) to process the format string and then call fwrite, which could be also what vfprintf does.

# Functions

#### fileDesc
`int getDirSz (DIR** dir)`\
 get dir size, dir is assumed to be valid\

`void fetchStats (char* path, fdDesc* ret)`\
 fetch stats for a given file descriptor and puts them in ret\

`int frPidPathFtFdInfo (char* pidPath, fdDesc** bff)`\
 given a pidPath (/proc/[pid]/fd) loop through all file descriptors and allocate a space in bff, then fill in all information in bff returns the size of bff, returns -1 if an error occured

`pidFdDesc* fetchAll (uid_t user)`\
fetch all open file descriptors for the current user if printall != 0 fetch all the open file descriptors returns a list pidFdDesc structs\

`pidFdDesc* fetchSingle (int pid)`\
fetch open file descriptors for a given pid the pid is not checked if it is acessible but if the pid <= 0, pid is set to the current processes pid\

`void destroyPidFdDesc (pidFdDesc* target)`\
recursivley free alloc'd memory

#### IO
`int printOut(FILE* txtOut, printMode mode, const char* format, ...)`\
print to the indicate FILE* pointed to by txtOut, using the mode (binary or ascii)\
printOut uses variadic functions to wrap printf and fwrite together\
\
if mode == p_stdout or mode == p_text print to the txtOut, using vfprintf (basically printf)\
if mode == p_binary use vsprintf to process the string and print to the file using fwrite\
\
txtOut - File to print to (assumed to be opened in the correct mode and already opened/valid)\
mode   - Specifies mode to print as (p_stdout is mostly redundant but was kept to not break things)\
format - format string (see printf docs)\
...    - see printf docs

#### main.c
`void printThresh (FILE* stream, printMode outputMode, pidFdDesc* in, int threshold)`\
print threshold\
prints and handles the threshold of all the file descriptors in (pidFdDesc* in) based on threshold\
\
stream     - file to print to\
outputMode - mode to print in\
in         - list of file descriptors\
threshold  - threshold\

print the table\
what to print is specified by the PRINT_* macros, and is formatted and printed to stream\
\
flags is taken in as a bitmask\
\
for example:\
PRINT_INODES | PRINT_FLNAME prints the inodes and filename\
PRINT_PROCID                prints the process id

#### misc
`int isNum (char* chk)`\
Check if chk is a number\
chk("70") == 1; chk("70sdsfd") == false; chk("slkdf98") == 0