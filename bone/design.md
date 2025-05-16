# Concept of `bone`
## Idea of `bone`
`bone` is based on a pretty common idea of _hot reloading_. Although there's nothing hot in `bone`, but just plain reloading. But why would one need this? Manual reloading ain't difficult. One would have to, in worst case scenario, send a `SIGINT` signal to the process and then press the up arrow key to get the previous command then press enter in the terminal to execute it. I wouldn't disagree either, its not difficult. But I do cherish the days of naive single threaded programing in VS C*de and the hollow feeling of hitting the `F5` key to run it, where all the magic is already done for me and i am sitting there like a clown waiting for the gods of compilations to bless me with the executable. The same goes for when I started with web dev. The only thing i had to do is press `<C-s>` and the _hot reloading_ would handle it all.

The concept of _hot reloading_ or _auto reloading_ is amazing in itself and I wanted to know more about it. What's a better way to learn something than by implementing it yourself.

## Why `bone`?
I've known about [nob.h](https://github.com/tsoding/nob.h) (reason: regular mista zozin viewa) and loved it's concpet. You want to build C? We gonna build it using C, and nothing else. What caught my attention was that once the program has been bootstraped, the next time, we only need to run it and it recompiles itself (sort of)! This gave me the idea to start of with something like _auto reloading_ for c/cpp. And `bone` is just `nob` but opposite, with an extra `e` for `execution`. Another reason for making it more of a _reloading_ type than a `nob` type is because of threading. Before `bone` I did not have any experience with threading in lower level on the Operating System. This was also meant to be a project to familiarize me with low level threading and process synchronisation.

# Architecture and Design of `bone`
## Introduction
This project deals with manipulation of _processes_, _threads_ and _terminal_ on _Linux_. The architecture of the project is subject to change as I am, myself, learning about afforementioned topics as i proceed to progress with this project. The current methods for manipulations might not be the best and optimum way, but I am very much open for improvements if one suggests any.

## Mechanism
When `bone` is provided a `srcFile`, as it would be obvious, verification and validation of the `srcFile` is performed. This process includes checking whether the executable for the given `srcFile` exists or not.

