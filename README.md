# fctree

## To run the code

```
cd Build
make
./firmcore_baseline -d dataset -m method
```

dataset(-d):

- example
- fao
- homo
- obamainisrael
- sacchcere
- so
- wiki


method(-m):

- serial (FirmCore Tree based solution)
- dfs

order(-o):

- 0 stand for (0, 1, 2, 3, 4 ...) default order
- 1 stand for numbe of edges from large to small
- 2 stand for number of edges from small to large

k(-k), only in the serial, parallel version can use:

- k: specify the output k

m(-m), only in the serial, parallel version can use:

- lmd: specify the output lambda 