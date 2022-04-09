# https://github.com/Amber-MD/cmake-buildscripts/blob/7efd69bcb2f1cd432fd0374c9b04b129e5b03577/FindTermcap.cmake

# Script to figure out which of the different terminal libraries (terminfo/termcap) to use to get the Terminfo functions
# We search for "tgetent", which is what the original Automake code (BASH_CHECK_LIB_TERMCAP) used.

# Defines the following variables: 
# TERMCAP_INCLUDE_DIR - include directory containing termcap headers (there are a few different ones)
# TERMCAP_LIBRARY   - library to link to get termcap functions
# TERMCAP_FOUND     - whether or not a library and header were found

set(TERMCAP_SYMBOL tgetent)


# check libraries

# skip if user defined on cache
if(NOT (DEFINED TERMCAP_LIBRARY AND EXISTS "${TERMCAP_LIBRARY}"))
  
  foreach(POSSIBLE_LIBRARY termcap tinfo ncurses curses gnutermcap)
    
    # on some systems (e.g. Anaconda's runtime), these libraries might exist but be missing the symbols we need.
    # so, we have to check that they have the needed symbol.
    string(TOUPPER ${POSSIBLE_LIBRARY} POSSIBLE_LIBRARY_UCASE)
    find_library(${POSSIBLE_LIBRARY_UCASE}_LIBRARY NAMES ${POSSIBLE_LIBRARY} DOC "Path to a Termcap candidate library")
    
    if(${POSSIBLE_LIBRARY_UCASE}_LIBRARY)
    
      check_library_exists(${${POSSIBLE_LIBRARY_UCASE}_LIBRARY} ${TERMCAP_SYMBOL} "" HAVE_TERMCAP_SYMBOL_IN_${POSSIBLE_LIBRARY_UCASE})
      
      if(HAVE_TERMCAP_SYMBOL_IN_${POSSIBLE_LIBRARY_UCASE})
        set(TERMCAP_LIBRARY ${${POSSIBLE_LIBRARY_UCASE}_LIBRARY})
        
        break()
      endif()
      
    endif()
  endforeach()
endif()

# --------------------------------------------------------------------

set(TERMCAP_INCLUDE_HINTS "")

# check headers
if(EXISTS "${TERMCAP_LIBRARY}")
  # try to work backwards from the library location
  get_filename_component(TERMCAP_LIBRARY_DIR "${TERMCAP_LIBRARY}" PATH)
  
  list(APPEND TERMCAP_INCLUDE_HINTS "${TERMCAP_LIBRARY_DIR}/../include")
endif()

find_path(TERMCAP_INCLUDE_DIR NAMES termcap.h curses.h term.h ncurses.h HINTS ${TERMCAP_INCLUDE_HINTS})

# --------------------------------------------------------------------

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Termcap FOUND_VAR TERMCAP_FOUND REQUIRED_VARS TERMCAP_LIBRARY TERMCAP_INCLUDE_DIR)
