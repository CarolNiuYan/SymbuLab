import java.util.*;
import java.io.IOException;
import java.io.BufferedReader;
import java.io.*;

import org.omg.CORBA.portable.InputStream;
    
public class RunCommandLine {
    public static void main (String[] args) {
        // Run each set of parameters for 30 times
        int  random_num = 0;
        for (int i = 0; i < 20; i++) {
            try {
                System.out.println(i+1);
				
                // Generate new config files
                ReplaceFileContents rfc = new ReplaceFileContents();
                rfc.replace(i+1);
				
                Runtime rt = Runtime.getRuntime();

                Process proc = rt.exec("./symbulation");
                java.io.InputStream stdin = proc.getInputStream();
                InputStreamReader isr = new InputStreamReader(stdin);
                BufferedReader br = new BufferedReader(isr);

                String line = null;
                System.out.println("<OUTPUT>");

                while ( (line = br.readLine()) != null)
                     System.out.println(line);
                
                proc.waitFor();
                proc.destroy();
		
            } catch(Exception e) {
                System.out.println(e.toString());
                e.printStackTrace();
            }
        }
    }
}
