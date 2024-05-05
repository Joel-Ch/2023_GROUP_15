# VR Base Station

## Table of Contents

- [Installation](#installation)
- [Usage](#usage)
- [Contributing](#contributing)
- [License](#license)

## Installation

To build and install this project, follow these steps:

1. Clone the repository

2. Install QT, download and build VTK with OpenVR
    Make sure They're added to the path

3. Create a build directory and navigate into it

4. Generate the build files using CMake
    You will need to specify the OpenVR folder in cmake

5. Build the project

## Usage

Running the file will bring up the ui, including various features such as a render window and a treeview.
Item options are accessed by right clicking on the items in the tree, including changing colours, disabling items and deleting items.
Adding items, folders and starting the vr is done by clicking the buttons in the toolbar.
After starting VR, filters can be added to individual items using the dropdown menus.

## Contributing

If you would like to contribute to this project, please follow these guidelines:

1. Fork the repository.
2. Create a new branch.
3. Make your changes and commit them.
4. Push your changes to your forked repository.
5. Submit a pull request.

## License

This project is licensed under the [MIT License](LICENSE).
