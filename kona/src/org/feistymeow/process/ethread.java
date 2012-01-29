
package org.feistymeow.process;

//still in progress...
// not compilable yet probably, 
// plus missing the timed features of ethread.


import java.io.BufferedOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.Reader;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * A simple java thread that hearkens back to the HOOPLE C++ ethread in features.
 *
 * @author Chris Koeritz
 */
class ethread implements Runnable
{
//fix below to use better way to get class object.
  private static Log c_logger = LogFactory.getLog(ethread.class);

  private volatile Thread c_RealThread = null;

  // the only variable on which both synchronize is the "thread finished" variable.
  public ethread()
  {
  }

  /**
   * Begins execution of the thread.
   */
  public void start()
  {
    if( null == this.c_RealThread )
    {
      this.c_RealThread = new Thread(this);
      this.c_RealThread.start();
    }
  }
  
  /**
   * Stops execution of the thread, or at least attempts to.
   */
  public void stop()
  {
    Thread goAway = c_RealThread;
    c_RealThread = null;
    if (null != goAway) { goAway.interrupt(); }
  }
  
  /**
   * Returns true if the thread isn't null.
   */
  public boolean threadRunning()
  {
    return (null != this.c_RealThread);
  }
  
  public void run()
  {
    if (false == threadRunning())
    {
      return; // stopped before it ever started
    }
    
  }

}
