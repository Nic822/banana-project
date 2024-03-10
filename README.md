# Banana Project

This is a mini-project for the software engineering course of the [FHGR B.Sc. Mobile Robotics](https://fhgr.ch/mr),
implemented by [Dominic Eicher](https://github.com/Nic822) and [Ralph Ursprung](https://github.com/rursprung).

This project analyses pictures of bananas to try and identify some information about them (e.g. their ripeness).

## Project Structure

The project consists of a library implementing the actual functionality and two applications, one feeding the library
with live pictures from an attached camera and one feeding it static images (mainly for manual testing).

## Building

To build this project you will need:

* A modern C++ compiler supporting C++20
* [CMake](https://cmake.org/) incl. CTest - this might well come included with your favourite IDE
* [vcpkg](https://vcpkg.io/)
