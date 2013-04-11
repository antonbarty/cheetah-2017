# Cheetah Installation Instructions

### Preparing for the build

To build cheetah you need cmake (and optionally ccmake). 

If you're on psexport you can just do:

    $ export PATH=~filipe/cmake/bin:${PATH}


Otherwise just ask your sysadmin to install it.

If your HDF5 is not installed in a standard location set the
HDF5_ROOT environment variable to point to it, e.g.:

    $ export HDF5_ROOT=${HOME}/local


You also need to have LCLS's ana available somewhere. If you're building 
at CFEL, LCLS or Uppsala this should already be available. 

If not you can try to use the scripts/download_psana.py to download it 
but you will likely have to do modifications. The best is probably to 
ask LCLS about getting a portable version of ana.



### Building and Installing

Now that you have cmake we can start the build:

- Create and go into a build directory:

        $ mkdir build
        $ cd build
 
- Run ccmake

        $ ccmake ..


- Press "c" to configure. 

- It's possible that you have to specify `ANA_RELEASE` manually. It should
point to the ana-current directory, for example on psexport it is 
`/reg/g/psdm/sw/releases/ana-current/`

- You can also specify the `CMAKE_INSTALL_PREFIX`. I set mine to `~/usr`
- If everything went well you should be able to
press "g" to generate the Makefiles.

- Now just run make. This will build things and place the result in the
build directory.

        $ make



- If you want to install just do.

        $ make install


### Notes on psana and shared libraries

- If you do not care for `LD_LIBRARY_PATHs` and `RPATHs` you can safely skip 
this section and everything should work as expected. Otherwise read on! 

Cheetah uses as the main backend psana. psana looks up and loads shared 
libraries at runtime according to the psana.cfg configuration files. If 
it cannot find the library specified it will exit with an error. The 
loaded dynamic libraries will themselves load other libraries, for 
example libcheetah_ana_mod loads libcheetah such that you end up with 
the following chain:

    psana->libcheetah_ana_mod->libcheetah->libhdf5(among others)

This can often cause headaches for the user if the libraries are not in 
standard locations (which they usually are not). One solution is to set
the `LD_LIBRARY_PATH` variable to include the directory of all the 
necessary libraries. Another option is to set the `RPATH` on the programs 
and libraries which tells them where to look for libraries. The `RPATH` 
takes precedence over the `LD_LIBRARY_PATH` and because it's hardwired
is less flexible.

When building cheetah all binaries inside the build directory will 
have the `RPATH` set to the location using during the linking stage. This 
means that you should be able to run them directly from the build 
directory without the need to set any `LD_LIBRARY_PATH`.
When you do `make install` the binaries are moved to their install 
directory and the `RPATH` is can be removed from the binaries. This 
means that on the one hand you now can control where they load the libraries from 
using the `LD_LIBRARY_PATH`, but on the other hand they will likely not 
find the required libraries if you do not specify any `LD_LIBRARY_PATH`.

If you wish to remove the `RPATH` from the installed binaries and you 
know what you're doing set `RPATH_ON_INSTALLED_BINS` to FALSE in 
`ccmake`.
