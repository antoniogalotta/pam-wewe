## Project Structure

This project is organized into the following main directories:

*   **`main/`**: Contains the core source code for the `pam-wewe` module.
    *   **`main/include/`**: Header files (`.h`) 
    *   **`main/src/`**: Source files (`.c`) 
*   **`debian/`**: Contains files required for building the Debian package (`.deb`).
*   **`test/`**: Contains unit tests and resources for testing the module.
    *   **`test/resources/`**: Sample configuration files used during testing.
    *   **`test/src/`**: Source code for the test suite.

### Compilation

After installing the dependencies, you can compile the project:

```bash
make
```

### Building Debian Package

To build the Debian package (`.deb` file):

```bash
make deb
```

## Building with Docker

You can also build the project using Docker. This method is useful if you don't want to install all the development dependencies on your host machine.

```bash
make docker-build
```