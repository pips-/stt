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
  task: task
  started at: Wed Oct  2 19:04:04 2019
  ended at: Wed Oct  2 19:05:04 2019
  duration(hours): 0.02

  task: readme@stt
  started at: Wed Oct  2 19:05:04 2019
  ended at: Wed Oct  2 19:09:28 2019
  duration(hours): 0.07

  task: publish@stt
  started at: Wed Oct  2 19:09:28 2019
  still running
  duration(hours): 0.11

Files
-----

If current working directory contains a `.ttimes` file it will be used
otherwise it will create or use `~/.ttimes`.

File format looks like:

	<start-unix-timestamp>;<end-unix-timestamp>;<task>

Zero value end timestamp means task is still running.
