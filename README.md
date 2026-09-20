# Network Learn



# Setup & Building

1. Clone the repository and initialize submodules:
    ```sh
    git clone --recursive https://github.com/ArturasDruteika/network_learn.git
    # or, if already cloned:
    git submodule update --init --recursive
    ```

2. Install build dependencies (Linux example):
    ```sh
    ./scripts/linux_setup.sh
    ```

3. Build the project:
    ```sh
    ./scripts/linux_build.sh
    ```

    Optionally, install the built project:
    ```sh
    ./scripts/linux_install.sh
    ```

4. Run the app (example):
    ```sh
    # EXAMPLE

    # On terminal 1:
    cd build/release/bin
    ./server_lesson_1

    # On terminal 2:
    cd build/release/bin
    ./client_lesson_1
    ```