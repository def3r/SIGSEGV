*https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl01.html*

Compile:
```sh
clang++ -g -O3 lang.cpp $(llvm-config --cxxflags --ldflags --system-libs --libs core)
```
