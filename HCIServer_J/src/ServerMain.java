import java.awt.image.BufferedImage;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.ObjectInputStream;
import java.io.OutputStreamWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Scanner;

import javax.imageio.ImageIO;

public class ServerMain {

	public static void main(String[] args) throws ClassNotFoundException, IOException {
		// TODO Auto-generated method stub
		String path ="E:/Desktop/HCI/target/target.jpg";//이미지 저장할 경로
		
        int port1 = 8200;//안드로이드
        int port2 = 1234;//딥러닝 서버
        int port3 = 8900;//햅틱 서버
        String ipD="";//딥러닝 서버
        String ipH="";//햅틱서버
        String result;

        
        ServerSocket sock = new ServerSocket(port1);
        
//        Socket socketD = new Socket(ipD,port2);
//        Socket socketH = new Socket(ipH,port3);
        
        
        while(true){
        	Socket socket = sock.accept();
		BufferedReader inD = null;
		BufferedWriter outD = null;
		
		
		BufferedWriter outH = null;
        
        //Socket로부터 InputStream을 등록
        InputStream is = socket.getInputStream();
        //등록한 InputStream을 ObjectInputStream방식으로 사용함
        final ObjectInputStream ois = new ObjectInputStream(is);
        
        //전송된 byte[] 데이터를 수신
        byte[] data = (byte[])ois.readObject();
        
        //이미지 저장부
        ByteArrayInputStream btis = new ByteArrayInputStream(data);
        BufferedImage bufferedImage =ImageIO.read(btis); 
        ImageIO.write(bufferedImage, "jpg", new File(path));

//		inD = new BufferedReader(new InputStreamReader(socketD.getInputStream()));
//		outD = new BufferedWriter(new OutputStreamWriter(socketD.getOutputStream()));
//      outH = new BufferedWriter(new OutputStreamWriter(socketH.getOutputStream()));
	
//		outD.write("Done");//파일저장 완료 메세지
//		outD.flush();
        
//		result = inD.readLine();//인지결과 수신
//        outH.write(result);//인지결과 햅틱 서버로 전송
//        outH.flush();
//		System.out.println(result);
		
        ois.close();
        is.close();
//        inD.close();
//        outD.close();
//        outH.close();
        
        }


	}

}
