# AcramAlpha
Mathematical equations' parser &amp; differentiator


## Compilation
- `cmake CMakeLists.txt ; make`.

If `ADVANCED_MODE` is defined in "*options.hpp*", equation simplifications are better.

> WARNING: `ADVANCED_MODE` may result in unexpected simplification if such operations like "* \* *" and "* - *" were redacted via scripts. Turing it off leads to only those simplifications defined in "*simplify.dat*".


## Running
- example: `./diff files/*.in`

The executable "*diff*" expects for a list of files containing mathematical equations as an argument. For each file the output is stored in the same directory as the file.

The output is a LaTeX file containing 2 formulas: the original equation `f(x)` and its differential `f'(x)`. Also the result of all equations are stored in the working directory in "*output.tex*".

Under Linux where packages `texlive` and `texlive-lang-cyrillic` are installed the LaTeX files will be automatically converted to PDF with removing the mediators.


## Preconditions
- Mathematically correct (*no 2*/4 or *).
- Contains only "* x *" as a variable.
- Using only operations specified in "*lib/scripts/*".

Any parsing difficulties will be reported though.

The examples are stored in "*files*" directory.


## Permissions
- Input file is not to have "*.in*" extension.
- Using spaces.
- Using unary "* - *" even in sequences.
- Using double numbersnot starting from point (*1, 1.2, 12e-1 - OK, .2 - NO*).

## Description

### Library (./lib)
- *Equation*: storage for mathematical formulas as binary tree.
- *scriptparser*: some weird functions based on parsing scripts using macro'.
- *TexFile*: creating a texfile with appropriate header and converting it.
- *system*: operating with OS environment (available only for Linux).

### Scripts (./lib/scripts)
- *functions.dat*: the most important one; specifies operations names for parsing and programm, priority, differential calculation and LaTeX formula generation.
- *simplify.dat*: specification upon degenerated cases for simple calculations (*x*0=0, sqrt(0)=0, etc.*). For unary functions the inverse one is also specified.
- *calculate.dat*: specifies how to calculate an operations when numeric constants provided as operands (*"/" not included as story fractions look better*).
- *duplex.dat*: specifies how to calculate an operations when operands provided are equal.
- *relation.dat*: specification upon operations' characteristics and co-relations (*like commutativity and associativity*).


## Issues
- Operations' relations are not used by their full potential, sketches are to be found in `Equation::simplify`.
- Convertion doesn't work not under Linux.
- Binary suffix operations like log(a, b) not supported - only prefic ones.
- Advanced mode implies certain prescription (+ ~ add, * ~ multiplicate, etc.) and therefore not common.
