package id.au.ringerc.testcase.as7.eclipselink;

import java.util.HashMap;
import java.util.Map;


import org.eclipse.persistence.logging.AbstractSessionLog;
import org.eclipse.persistence.logging.SessionLogEntry;
import org.jboss.logging.Logger;
import org.jboss.logging.Logger.Level;

//
// De-Guava-ized version of code from:
// https://community.jboss.org/wiki/HowToUseEclipseLinkWithAS7
//
public class JBossLogger extends AbstractSessionLog {


          private Map<String, Logger> loggers = new HashMap<String, Logger>();
          
          @Override
          public void log(SessionLogEntry sessionLogEntry) {
                    Logger logger = getLoggerForCategory(sessionLogEntry.getNameSpace());
                    Level level = convertLevelIntToEnum(sessionLogEntry.getLevel());
                    String message = formatMessage(sessionLogEntry);

                    logger.log(level, message);
          }

          @Override
          public boolean shouldLog(int level, String category) {
                    return getLoggerForCategory(category).isEnabled(convertLevelIntToEnum(level));
          }

          private Logger getLoggerForCategory(String category) {
                    Logger logger = loggers.get(category);
                    if (logger == null) {
                              logger = Logger.getLogger("org.eclipse.persistence", (category == null ? "" : category) );
                              loggers.put(category, logger);
                    }
                    return logger;
          }


          private Level convertLevelIntToEnum(int level) {
                    switch (level) {
                              case SEVERE:
                                        return Level.FATAL;
                              case WARNING:
                                        return Level.WARN;
                              case CONFIG:
                              case INFO:
                                        return Level.INFO;
                              case FINE:
                                        return Level.DEBUG;
                              case FINER:
                              case FINEST:
                                        return Level.TRACE;
                              default:
                                        getLoggerForCategory("logging").warnv("Received message for log level {0}, but no such level is defined in SessionLog! Logging at INFO level...", level);
                                        return Level.INFO;
                    }
          }


}
