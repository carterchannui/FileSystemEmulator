# File System Emulator

Utilize various operations and commands to navigate and alter a simulated filesystem.
## Usage
Compile files:
```
make
```

Launch the emulator within given filesystem with an inode limit:
```
./main [INODE-LIMIT] fs
```

## Valid Operations File Commands
The following displays the emulator's valid commands with proper syntax.
```
// Change directory.
cd [DIRECTORY]

// List directory contents.
ls [FILE]...

// Make directories.
mkdir [DIRECTORY]...

// Change file timestamps.
touch [FILE]...

// Remove (delete) files, directories, device nodes and symbolic links .
rm [OPTION]... [FILE]...
```

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please make sure to update tests as appropriate.

## License
[MIT](https://choosealicense.com/licenses/mit/)
