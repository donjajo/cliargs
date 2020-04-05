# cliargs
Read CLI arguments into your C application

## Sample
This can be found in [cliargs_test.c](https://github.com/donjajo/cliargs/blob/master/tests/cliargs_test.c)
```sh
gcc tests/cliargs_test.c -Wall -o test
```

## Supports
- Long & Short arguments
```sh
app -l --long-name
```
- Group short arguments
```sh
app -lnR value_of_R
```
- Required Arguments

Wrote this learning C, improvements and critics are welcome :)
