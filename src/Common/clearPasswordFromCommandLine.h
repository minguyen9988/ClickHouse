#pragma once

/** If there are --password=... / --password ... or --jwt=... / --jwt ... arguments in command line,
  * replace their values with zero bytes.
  * This is needed to prevent password and token exposure in 'ps' and similar tools.
  */
void clearPasswordFromCommandLine(int argc, char ** argv);
