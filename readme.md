# MapReduce

## Motivation/About

MapReduce is a

## Example/Testing

### Usage
Run the executable with two arguments--in order they are the path of the file to be processeed and your public IP, both strings.

### Releases
To simply run a built executable of the MapReduce demo, go to the Releases tab and download the release for your OS (Windows is as of yet not supported).

### Building from Source
First, clone the repository with `git clone https://github.com/richardfeynmanrocks/mapreduce.git`.

For MacOS, run the following commands inside the project directory:
```
gcc example.c mapreduce.c worker.c server.c -g -w -o example
./example <PATH-TO-INPUT> <YOUR-PUBLIC-IP>`
```

## Known Issues
* Last split piece of file is somehow unreadable and is essentially ignored 
* Coordination between separate machines is essentially nonexistent

## Execution

## Future Features
