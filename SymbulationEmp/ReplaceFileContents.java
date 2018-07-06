import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

public class ReplaceFileContents {
		   public void replace(int random_num) {
			   
			  // Change pathname according to computers
		      String oldFileName = "./SymSettings.cfg";
		      String tmpFileName = "./TempSettings.cfg";

		      BufferedReader br = null;
		      BufferedWriter bw = null;
		      try {
		         br = new BufferedReader(new FileReader(oldFileName));
		         bw = new BufferedWriter(new FileWriter(tmpFileName));
		         String line;
		         while ((line = br.readLine()) != null) {
		            if (line.contains("SEED"))
		               line = "set SEED " + random_num;
		            bw.write(line+"\n");
		         }
		      } catch (Exception e) {
		         return;
		      } finally {
		         try {
		            if(br != null)
		               br.close();
		         } catch (IOException e) {
		            //
		         }
		         try {
		            if(bw != null)
		               bw.close();
		         } catch (IOException e) {
		            //
		         }
		      }
		      // Once everything is complete, delete old file..
		      File oldFile = new File(oldFileName);
		      oldFile.delete();

		      // And rename tmp file's name to old file name
		      File newFile = new File(tmpFileName);
		      newFile.renameTo(oldFile);

		   }
}
