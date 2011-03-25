Bug report reference: https://issues.jboss.org/browse/SEAMFACES-118

asadmin deploy --contextroot justtesting target/glassfish-seamfaces-flash-injection.war

Deployment log and failure for this test case, deploying to Glassfish 3.1:
remote failure: Error occurred during deployment: Exception while loading the app : WELD-001409 Ambiguous dependencies for type [Flash] with qualifiers [@Default] at injection point [[field] @Inject private com.mycompany.glassfishseamfacesflashinjection.InjectionTest.flash]. Possible dependencies [[Producer Method [Flash] with qualifiers [@Any @Default] declared as [[method] @Produces @RequestScoped public org.jboss.seam.faces.environment.FacesFlashProducer.getFlash()], Producer Method [Flash] with qualifiers [@Any @Default] declared as [[method] @Produces @RequestScoped public org.jboss.seam.faces.context.FlashProducer.getFlash()]]]. Please see server.log for more details.

Server log:

[#|2011-03-25T10:03:38.233+0800|INFO|glassfish3.1|javax.enterprise.system.container.web.com.sun.enterprise.web|_ThreadID=67;_ThreadName=Thread-1;|WEB0671: Loading application [glassfish-seamfaces-flash-injection] at [/justtesting]|#]

[#|2011-03-25T10:03:38.240+0800|SEVERE|glassfish3.1|javax.enterprise.system.core.com.sun.enterprise.v3.server|_ThreadID=67;_ThreadName=Thread-1;|Exception while loading the app|#]

[#|2011-03-25T10:03:38.848+0800|SEVERE|glassfish3.1|javax.enterprise.system.tools.admin.org.glassfish.deployment.admin|_ThreadID=67;_ThreadName=Thread-1;|Exception while loading the app : WELD-001409 Ambiguous dependencies for type [Flash] with qualifiers [@Default] at injection point [[field] @Inject private com.mycompany.glassfishseamfacesflashinjection.InjectionTest.flash]. Possible dependencies [[Producer Method [Flash] with qualifiers [@Any @Default] declared as [[method] @Produces @RequestScoped public org.jboss.seam.faces.environment.FacesFlashProducer.getFlash()], Producer Method [Flash] with qualifiers [@Any @Default] declared as [[method] @Produces @RequestScoped public org.jboss.seam.faces.context.FlashProducer.getFlash()]]]
org.jboss.weld.exceptions.DeploymentException: WELD-001409 Ambiguous dependencies for type [Flash] with qualifiers [@Default] at injection point [[field] @Inject private com.mycompany.glassfishseamfacesflashinjection.InjectionTest.flash]. Possible dependencies [[Producer Method [Flash] with qualifiers [@Any @Default] declared as [[method] @Produces @RequestScoped public org.jboss.seam.faces.environment.FacesFlashProducer.getFlash()], Producer Method [Flash] with qualifiers [@Any @Default] declared as [[method] @Produces @RequestScoped public org.jboss.seam.faces.context.FlashProducer.getFlash()]]]
	at org.jboss.weld.bootstrap.Validator.validateInjectionPoint(Validator.java:309)
	at org.jboss.weld.bootstrap.Validator.validateBean(Validator.java:139)
	at org.jboss.weld.bootstrap.Validator.validateRIBean(Validator.java:162)
	at org.jboss.weld.bootstrap.Validator.validateBeans(Validator.java:385)
	at org.jboss.weld.bootstrap.Validator.validateDeployment(Validator.java:371)
	at org.jboss.weld.bootstrap.WeldBootstrap.validateBeans(WeldBootstrap.java:390)
	at org.glassfish.weld.WeldDeployer.event(WeldDeployer.java:190)
	at org.glassfish.kernel.event.EventsImpl.send(EventsImpl.java:128)
	at org.glassfish.internal.data.ApplicationInfo.start(ApplicationInfo.java:298)
	at com.sun.enterprise.v3.server.ApplicationLifecycle.deploy(ApplicationLifecycle.java:461)
	at com.sun.enterprise.v3.server.ApplicationLifecycle.deploy(ApplicationLifecycle.java:240)
	at org.glassfish.deployment.admin.DeployCommand.execute(DeployCommand.java:370)
	at com.sun.enterprise.v3.admin.CommandRunnerImpl$1.execute(CommandRunnerImpl.java:355)
	at com.sun.enterprise.v3.admin.CommandRunnerImpl.doCommand(CommandRunnerImpl.java:370)
	at com.sun.enterprise.v3.admin.CommandRunnerImpl.doCommand(CommandRunnerImpl.java:1067)
	at com.sun.enterprise.v3.admin.CommandRunnerImpl.access$1200(CommandRunnerImpl.java:96)
	at com.sun.enterprise.v3.admin.CommandRunnerImpl$ExecutionContext.execute(CommandRunnerImpl.java:1247)
	at com.sun.enterprise.v3.admin.CommandRunnerImpl$ExecutionContext.execute(CommandRunnerImpl.java:1235)
	at com.sun.enterprise.v3.admin.AdminAdapter.doCommand(AdminAdapter.java:465)
	at com.sun.enterprise.v3.admin.AdminAdapter.service(AdminAdapter.java:222)
	at com.sun.grizzly.tcp.http11.GrizzlyAdapter.service(GrizzlyAdapter.java:168)
	at com.sun.enterprise.v3.server.HK2Dispatcher.dispath(HK2Dispatcher.java:117)
	at com.sun.enterprise.v3.services.impl.ContainerMapper.service(ContainerMapper.java:234)
	at com.sun.grizzly.http.ProcessorTask.invokeAdapter(ProcessorTask.java:822)
	at com.sun.grizzly.http.ProcessorTask.doProcess(ProcessorTask.java:719)
	at com.sun.grizzly.http.ProcessorTask.process(ProcessorTask.java:1013)
	at com.sun.grizzly.http.DefaultProtocolFilter.execute(DefaultProtocolFilter.java:225)
	at com.sun.grizzly.DefaultProtocolChain.executeProtocolFilter(DefaultProtocolChain.java:137)
	at com.sun.grizzly.DefaultProtocolChain.execute(DefaultProtocolChain.java:104)
	at com.sun.grizzly.DefaultProtocolChain.execute(DefaultProtocolChain.java:90)
	at com.sun.grizzly.http.HttpProtocolChain.execute(HttpProtocolChain.java:79)
	at com.sun.grizzly.ProtocolChainContextTask.doCall(ProtocolChainContextTask.java:54)
	at com.sun.grizzly.SelectionKeyContextTask.call(SelectionKeyContextTask.java:59)
	at com.sun.grizzly.ContextTask.run(ContextTask.java:71)
	at com.sun.grizzly.util.AbstractThreadPool$Worker.doWork(AbstractThreadPool.java:532)
	at com.sun.grizzly.util.AbstractThreadPool$Worker.run(AbstractThreadPool.java:513)
	at java.lang.Thread.run(Thread.java:636)
|#]

