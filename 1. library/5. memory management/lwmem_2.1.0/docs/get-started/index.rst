.. _getting_started:

Getting started
===============

Getting started may be the most challenging part of every new library.
This guide is describing how to start with the library quickly and effectively

.. _download_library:

Download library
^^^^^^^^^^^^^^^^

Library is primarly hosted on `Github <https://github.com/MaJerle/lwmem>`_.

You can get it by:

* Downloading latest release from `releases area <https://github.com/MaJerle/lwmem/releases>`_ on Github
* Cloning ``main`` branch for latest stable version
* Cloning ``develop`` branch for latest development

Download from releases
**********************

All releases are available on Github `releases area <https://github.com/MaJerle/lwmem/releases>`_.

Clone from Github
*****************

First-time clone
""""""""""""""""

This is used when you do not have yet local copy on your machine.

* Make sure ``git`` is installed.
* Open console and navigate to path in the system to clone repository to. Use command ``cd your_path``
* Clone repository with one of available options below

  * Run ``git clone --recurse-submodules https://github.com/MaJerle/lwmem`` command to clone entire repository, including submodules
  * Run ``git clone --recurse-submodules --branch develop https://github.com/MaJerle/lwmem`` to clone `development` branch, including submodules
  * Run ``git clone --recurse-submodules --branch main https://github.com/MaJerle/lwmem`` to clone `latest stable` branch, including submodules

* Navigate to ``examples`` directory and run favourite example

Update cloned to latest version
"""""""""""""""""""""""""""""""

* Open console and navigate to path in the system where your repository is located. Use command ``cd your_path``
* Run ``git pull origin main`` command to get latest changes on ``main`` branch
* Run ``git pull origin develop`` command to get latest changes on ``develop`` branch
* Run ``git submodule update --init --remote`` to update submodules to latest version

.. note::
	This is preferred option to use when you want to evaluate library and run prepared examples.
	Repository consists of multiple submodules which can be automatically downloaded when cloning and pulling changes from root repository.

Add library to project
^^^^^^^^^^^^^^^^^^^^^^

At this point it is assumed that you have successfully download library, either cloned it or from releases page.
Next step is to add the library to the project, by means of source files to compiler inputs and header files in search path

* Copy ``lwmem`` folder to your project, it contains library files
* Add ``lwmem/src/include`` folder to `include path` of your toolchain. This is where `C/C++` compiler can find the files during compilation process. Usually using ``-I`` flag
* Add source files from ``lwmem/src/`` folder to toolchain build. These files are built by `C/C++` compiler. CMake configuration comes with the library, allows users to include library in the project as **subdirectory** and **library**.
* Copy ``lwmem/src/include/lwmem/lwmem_opts_template.h`` to project folder and rename it to ``lwmem_opts.h``
* Build the project

Configuration file
^^^^^^^^^^^^^^^^^^

Configuration file is used to overwrite default settings defined for the essential use case.
Library comes with template config file, which can be modified according to needs.
and it should be copied (or simply renamed in-place) and named ``lwmem_opts.h``

.. note::
    Default configuration template file location: ``lwmem/src/include/lwmem/lwmem_opts_template.h``.
    File must be renamed to ``lwmem_opts.h`` first and then copied to the project directory where compiler
    include paths have access to it by using ``#include "lwmem_opts.h"``.

List of configuration options are available in the :ref:`api_lwmem_opt` section.
If any option is about to be modified, it should be done in configuration file

.. literalinclude:: ../../lwmem/src/include/lwmem/lwmem_opts_template.h
    :language: c
    :linenos:
    :caption: Template configuration file

.. note::
    If you prefer to avoid using configuration file, application must define
    a global symbol ``LWMEM_IGNORE_USER_OPTS``, visible across entire application.
    This can be achieved with ``-D`` compiler option.

Minimal example code
^^^^^^^^^^^^^^^^^^^^

To verify proper library setup, minimal example has been prepared.
Run it in your main application file to verify its proper execution

.. literalinclude:: ../examples_src/example_minimal.c
    :language: c
    :linenos:
    :caption: Absolute minimum example