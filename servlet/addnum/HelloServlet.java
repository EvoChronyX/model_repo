import java.io.*;
import jakarta.servlet.*;
import jakarta.servlet.http.*;

public class HelloServlet extends HttpServlet {

    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {

        response.setContentType("text/html");
        PrintWriter out = response.getWriter();
        String firstNumber = request.getParameter("a");
        String secondNumber = request.getParameter("b");

        out.println("<html><body>");

        int a = Integer.parseInt(firstNumber);
        int b = Integer.parseInt(secondNumber);
        int result = a + b;

        out.println("<h1>Hello from Apache Tomcat 10!</h1>");
        out.println("<h1>" + result + "</h1>");

        out.println("</body></html>");
    }
}
