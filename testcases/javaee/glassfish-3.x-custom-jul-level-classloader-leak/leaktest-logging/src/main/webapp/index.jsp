<%@page contentType="text/html" pageEncoding="UTF-8"%>
<!DOCTYPE html>
<html>
    <head>
        <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
        <title>JSP Page</title>
    </head>
    <body>
        <jsp:useBean class="com.mycompany.leaktest2.TestBean" scope="request" id="testBean"/>
        
        <h1><jsp:getProperty name="testBean" property="hello"/> from JSP!</h1>
        
    </body>
</html>
