package org.feistymeow.process;

//////////////
//Name   : ThreadSpawnerAndWatcher
//Author : Chris Koeritz
//Rights : Copyright (c) 2012-$now By University of Virginia
//////////////
//This file is free software; you can modify/redistribute it under the terms
//of the Apache License v2.0: http://www.apache.org/licenses/LICENSE-2.0
//Feel free to send updates to: [ koeritz@virginia.edu ]
//////////////


// test app by Chris Koeritz.

import java.io.IOException;
import java.lang.ProcessBuilder;
import java.lang.Process;
import java.util.Vector;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.log4j.PropertyConfigurator;

class Defaults
{
  public static int CONCURRENT_LAUNCHES = 1000;
};

// will be spawned multiply and each one will check its one process.
class ThreadSpawnerAndWatcher implements Runnable
{
  private static Log c_logger = LogFactory.getLog(ThreadSpawnerAndWatcher.class);

  private volatile Thread myThread;

  private Vector<String> cmdline;

  private ProcessBuilder procbuild;

  @SuppressWarnings("unchecked")
  public ThreadSpawnerAndWatcher(Vector<String> command_line)
  {
    cmdline = (Vector<String>) command_line.clone();
    procbuild = new ProcessBuilder(cmdline);
  }

  /**
   * Launches the thread and waits for its results.
   */
  public void start()
  {
    if (null == this.myThread) {
      this.myThread = new Thread(this);
      this.myThread.start();
    }
  }

  /**
   * Stops the execution of the ProcessWatchingThread - or tries to.
   */
  public void stop()
  {
    Thread goAway = myThread;
    myThread = null;
    if (null != goAway) {
      goAway.interrupt();
    }
  }

  /**
   * Returns true if the thread isn't null, i.e. it is still running.
   */
  public boolean threadRunning()
  {
    return (null != this.myThread);
  }

  public void run()
  {
    if (false == threadRunning()) {
      return; // stopped before it ever started
    }

    try {
      // c_logger.info("about to start process: " + cmdline);
      Process p = procbuild.start();
      p.waitFor();
      // c_logger.info("returned from awaiting process: " + cmdline);
    } catch (IOException e) {
      c_logger.debug("thread caught io exception on: " + cmdline);
    } catch (InterruptedException ie) {
      c_logger.debug("thread interrupted for: " + cmdline);
    } finally {
      this.myThread = null; // thread is exiting
    }
  }

  static public void main(String[] args) throws Throwable
  {
    PropertyConfigurator.configure("log4j.properties"); // hard-coded for
                                                        // eclipse based run.

    Vector<String> cmds = new Vector<String>();

    cmds.add("/bin/echo");
    cmds.add("hello jupiter");

    // cmds.add("/bin/sleep");
    // cmds.add("10");

    Vector<ThreadSpawnerAndWatcher> watchers = new Vector<ThreadSpawnerAndWatcher>();

    c_logger.info("revving up the process launching test.");

    // create all the threads and get them ready to go.
    for (int i = 0; i < Defaults.CONCURRENT_LAUNCHES; i++) {
      ThreadSpawnerAndWatcher newby = new ThreadSpawnerAndWatcher(cmds);
      watchers.add(newby);
    }

    // randomize start order?
    // now start all the threads, which will cause all our process launches.
    for (int i = 0; i < Defaults.CONCURRENT_LAUNCHES; i++) {
      ThreadSpawnerAndWatcher curr = watchers.get(i);
      curr.start();
    }

    // now wait for them all to finish. if we never get out, then there's a bug
    // someplace.
    for (int i = 0; i < Defaults.CONCURRENT_LAUNCHES; i++) {
      ThreadSpawnerAndWatcher curr = watchers.get(i);
      while (curr.threadRunning()) {
        try {
          Thread.sleep(100);
        } catch (InterruptedException e) {
          // ignore for now.
        }
      }
    }

    // currently test will never come back out to finish, if there's a failure
    // seen.
    c_logger.info("Test Succeeded: all spawned processes came back as expected.");
    System.exit(0);
  }
}
