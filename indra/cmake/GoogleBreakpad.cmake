# -*- cmake -*-
include(Prebuilt)

# <FS:ND> We only ever need google breakpad when crash reporting is used
if(RELEASE_CRASH_REPORTING OR NON_RELEASE_CRASH_REPORTING)

if (STANDALONE)
  set(BREAKPAD_EXCEPTION_HANDLER_FIND_REQUIRED ON)
  include(FindGoogleBreakpad)
else (STANDALONE)
  use_prebuilt_binary(google_breakpad)
  if (DARWIN)
    set(BREAKPAD_EXCEPTION_HANDLER_LIBRARIES exception_handler)
  endif (DARWIN)
  if (LINUX)
    set(BREAKPAD_EXCEPTION_HANDLER_LIBRARIES breakpad_client)
  endif (LINUX)
  if (WINDOWS)
    set(BREAKPAD_EXCEPTION_HANDLER_LIBRARIES exception_handler crash_generation_client common)
  endif (WINDOWS)
endif (STANDALONE)

# <FS:ND> Otherwise just disable it
else(RELEASE_CRASH_REPORTING OR NON_RELEASE_CRASH_REPORTING)
  add_definitions( -DND_NO_BREAKPAD )
endif(RELEASE_CRASH_REPORTING OR NON_RELEASE_CRASH_REPORTING)
# </FS:ND>