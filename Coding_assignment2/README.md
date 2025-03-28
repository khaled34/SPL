# **Polymorphic Shell** 🐚

A configurable shell implementation in C with multiple **flavors** (**femto, pico, nano, and micro**), each offering different levels of functionality.

## 🚀 **Overview**

This project provides a **polymorphic shell**, allowing you to build different versions with varying feature sets. The shell **name changes dynamically** based on the selected flavor.

## 🔧 **Building the Shell**

To build the shell, use:

```sh
make --help
```

This will guide you through the available build options and configurations.

The output binary will be:

```sh
<flavor_name>_shell
```

For example, building the **nano** flavor will produce:

```sh
nano_shell
```

## 🛠 **Shell Flavors and Features**

The shell can be built in **four different flavors**, each adding more functionality:

| **Flavor** | **Features**                                                    |
| ---------- | ----------------------------------------------------------------|
| **Femto**  | ✅ Supports only `echo` and `exit`                              |
|            | ❌ No environmental variable support                            |
|            | ❌ No redirection support                                       |
| **Pico**   | ✅ Adds support for `pwd` and `cd`                              |
| **Nano**   | ✅ Supports all Unix commands                                   |
|            | ✅ Supports environmental variables handling                    |
| **Micro**  | ✅ Supports **input/output/error redirection** (`<`, `>`, `2>`) |

---

## 📌 **Usage**

After building your shell, run it with:

```sh
./<flavor_name>_shell
```

For example:

```sh
./micro_shell
```

### **Femto Shell Example**

```sh
$ ./femto_shell.out
[prompt]$ echo Hello 
Hello
```

### **Nano Shell Example (Supports Environmental Variables)**

```sh
$ ./nano_shell.out
[prompt]$ VAR=3
[prompt]$ echo $VAR
3
```

### **Micro Shell Example (Supports Redirection)**

```sh
$ ./micro_shell.out
[prompt]$ echo "This is a test" > output.txt
[prompt]$ cat output.txt
This is a test
```

---

## 🤝 **Contributing**

I welcome contributions! To contribute:

1. Fork the repository
2. Create a new branch (`git checkout -b feature-xyz`)
3. Make your changes and commit (`git commit -m "Add feature xyz"`)
4. Push to the branch (`git push origin feature-xyz`)
5. Open a **Pull Request**

---
