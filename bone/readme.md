# Build On Execution
**`bone`** (build on execution) is an auto reloader / file watcher that rebuilds and re-runs the project once any dependencies are modiefied.

Currently supports C, C++ only

---

### Compilation:
> [!NOTE]
> *This is a Linux exclusive project.*

Build `bone.cpp` with `g++` or any of your fav compiler
```python
g++ bone.cpp -o bone
```

Now, to execute any file, simply run it with bone:
```python
./bone filename.c
```
Anytime this `filename.c` file is modified, `bone` will automatically build it and execute it.

### Keybinds:
Once the program exits, `bone` enters the interactive mode! To open help press `h` or `H` to see all the keybinds to control `bone`

## `nob.h`
Although very different from the implementation as well as concept of `bone`, this project is highly inspired from 
[nob.h](https://github.com/tsoding/nob.h) by `tsodin`

## TODO:
- [ ] Add multi file support
- [ ] Add a config file
- [ ] Build & monitor a cmake project
- [ ] Improve performance
- [ ] Extend to win
- [x] Add restart, quit, etc keybinds
- [ ] Expand to other langs that i use

- [ ] Tests and auto bffrs?
example:
```c
// # "1"
// # "string\tfine\n"
```
