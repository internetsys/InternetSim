# InternetSim

### Prerequisites

Make sure you have installed the following:
1. A C++17 compatible C++ compiler
2. [MongoDB](https://www.mongodb.com/docs/manual/installation/)
3. [MongoDB C++ Driver](https://www.mongodb.com/docs/languages/cpp/cpp-driver/current/get-started/)
4. Python's [Cython] package:

```bash
pip install Cython
```

### Configuration

You can modify the configuration using the `config.ini` file.

### Usage

Write a simulation description file, for example, refer to the two example files in the `description` directory.

Run the script to compile:

```bash
python3 setup.py build_ext --inplace
```

Execute the program with the following command:
```bash
python3 Main.py
```