import java.io.*;
import java.sql.*;
import javax.servlet.*;
import javax.servlet.http.*;
import javax.servlet.annotation.WebServlet;

@WebServlet("/students")
public class StudentServlet extends HttpServlet {

    protected void doGet(HttpServletRequest req,
                         HttpServletResponse res)
            throws IOException {

        res.setContentType("text/html");
        PrintWriter out = res.getWriter();

        try {
            Class.forName("com.mysql.cj.jdbc.Driver");

            Connection con = DriverManager.getConnection(
                "jdbc:mysql://localhost:3306/college",
                "root",
                "YOUR_MYSQL_PASSWORD"
            );

            Statement st = con.createStatement();
            ResultSet rs = st.executeQuery(
                "SELECT * FROM students"
            );

            out.println("<h2>Student Records</h2>");
            out.println("<table border='1'>");
            out.println("<tr><th>ID</th><th>Name</th>"
                      + "<th>Department</th></tr>");

            while (rs.next()) {
                out.println("<tr>");
                out.println("<td>" + rs.getInt("id") + "</td>");
                out.println("<td>" + rs.getString("name") + "</td>");
                out.println("<td>" + rs.getString("department") + "</td>");
                out.println("</tr>");
            }

            out.println("</table>");

            rs.close();
            st.close();
            con.close();

        } catch (Exception e) {
            out.println("Error: " + e.getMessage());
        }
    }
}