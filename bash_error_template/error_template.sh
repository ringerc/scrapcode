#!/bin/bash

#
#  -e: Trap when any command returns nonzero (failed) and exit.
#  -u: Trap and exit with an error if any undefined variable is referenced
#  -E: Inherit traps within functions
#
#  -o pipefail: If any part of a pipe fails, the pipe as a whole returns a
#     the failure code of the first failing part of the pipe. Without this
#     a pipe succeeds if the last command succeeds.
#
#
# Scripting for -e mode
# ----------------------
# When running with -e, make sure to handle any commands you expect to return
# nonzero with one of these patterns:
#
# if command args
# then
#   :  # do nothing or on-success actions here
# else
#   ecode=$?
#   ... handle error here ...
# fi
#
#
# if cmdoutput=$(command args)
# then
#   ... do what to do if it worked ...
# else
#   ecode=$?
#   ... do what to do if it failed...
# fi
#
#
# ecode=0
# command args || ecode=$?
# if [[ "$ecode" -ne 0 ]]
# then
#    ... handle error here ...
# fi
#
# Scripting for -u mode
# ---------------------
#
# If running with -u, you can't just use $1 if $1 may not be defined, etc. If a
# var may not be defined you have to conditionally access it.
#
# Typical patterns:
#
# ${var:-"default here"}
#
# ${var:?"error message if undefined"}
#
# ${var:+"only emitted if $var is defined"}
#
# if [[ -n "${var:-}" ]]
# then
#    ... $var defined ...
# fi
#
# You should generally pre-declare any globals.
#
# Take particular care with arrays. You cannot "${arrayvar[@]}" a possibly-undefined
# array. You have to use this icky spelling:
#
# "${#arrayvar[@]:+"${arrayvar[@]"}"
#
#
# Function variable scopes
# ------------------------
#
# Make a habit of using "local" in your functions to protect variable scopes. E.g. in this
# exmple, "$arg" won't be defined outside of function foo():
#
# function foo() {
#     local arg="${1:-default}"
#     do_something_with $arg
# }
# foo "${global}"
#
# To return multiple values from functions and return efficiently, use output globals.
# It's gross, but so is bash. E.g.:
#
# function bar() {
#    local x="${1:-}"
#    bar_out="$(( $x ++ ))"
# }
# bar 1
# echo ${bar_out}
#
# While we are here
# -----------------
#
# You can use 'printf' to set dynamic output variables.
#
#   printf -v $VARNAME "whatever value"
#
# and "${!VARNAME}" to access them by name-reference.
#
# But it's better to use "declare -A" and associative arrays where possible,
# since they're less awful than dynamic variable names.
#
# If you want to preserve spaces and quoting while building up a command
# use an array and "${x[@]}" expansion, e.g.
#
# declare -a cmd_and_args=("cmd" "arg1" "arg 2 kept together")
# cmd_and_args+=("arg 3 is not split by spaces")
# # run it, preserving quoting and arg groupings
# "${cmd_and_args[@]}"
#
# The 'read' command is great for simply splitting up input, and can
# be used with a here-string "<<<".
#
# read -r VAR1 VAR2 VAR3 <<<"var1input var2input var3input"
#
# It respects IFS so you can use it to split on tabs etc.
#




set -e -u -E -o pipefail

if [ -n "${DEBUG:-}" ]
then
  set -x -o functrace
fi

# Backtrace an error and then exit with the the original exit code
#
function trap_err() {
  filename=${BASH_SOURCE[1]:+"${BASH_SOURCE[1]}"}
  fileline=${BASH_LINENO[$lineno]:+"${BASH_LINENO[$lineno]}"}
  echo 1>&2 "ERROR: unexpected exit code $ecode at $filename:$lineno"
   echo "Backtrace is:"
   i=0
   while btline=$(caller $i)
   do
     if ! read -r line sub file <<<"$btline"
     then
        echo "    $btline"
     elif [[ "$sub" == "main" || "${sub}" == "" ]]
     then
       echo "    $file:$line"
     else
       echo "    $file:$line $sub()"
     fi
     i=$((i+1))
   done
  exit $ecode
}

# This trap runs on anything that would cause an exit due to -e
#
# It does not run on "exit n" for nonzero exit, or on -u traps
# or other internal shell errors.
#
# If you want that, you can add another trap on EXIT.
#
trap 'ecode=$?;lineno=$LINENO;set +x;trap_err' ERR

# Example of an exit trap.
#
# If you want to use this you must also put a "trap EXIT" before all your
# intentional exits, e.g.
#
#    trap EXIT
#    exit 1
#
trap 'ecode=$?;lineno=$LINENO;set +x;trap EXIT;trap_err' EXIT

# For testing. Source this file, then call foo() in the other file, maybe
# within a few nested functions, or maybe via sourcing some other files. also
# try ref'ing an undefined var
#
#function foo() {
#  false
#}
