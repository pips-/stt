stt
====

Simple Time Tracker

Install
-------

  # make clean install

Usage
-----

  $ stt -a task # start task
  $ stt -s      # stop task
  $ stt [-l]    # print report

Files
-----

If current working directory contains a `.ttimes` file it will be used
otherwise it will create or use `~/.ttimes`.

File format looks like:

	<start-unix-timestamp>;<end-unix-timestamp>;<task>

Zero value end timestamp means task is still running.
