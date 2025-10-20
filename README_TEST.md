## Instructions for testing:

First export ```BUILDPATH``` variable as the path to your build folder.
```
export BUILDPATH=./path/to/folder/where/binary/is/located
```
Note: Paths may vary, on some systems, paths lead to only build, but for others, they can be build/Debug or build/Release. 

## Experiments:

1. Bare server with many clients:

```

./launch_bare_server

```
with 
```

./stresstest_client

```


2. Server with many dynamic entities and one/many clients:

```

./stresstest_server

```
with
```
./launch_client
```
**OR**
```
./stresstest_client
```

3. (TODO) Server/client update Frequency variation.