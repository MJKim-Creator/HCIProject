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
		String path ="E:/Desktop/HCI/target/target.jpg";//�̹��� ������ ���
		
        int port1 = 8200;//�ȵ���̵�
        int port2 = 1234;//������ ����
        int port3 = 8900;//��ƽ ����
        String ipD="";//������ ����
        String ipH="";//��ƽ����
        String result;

        
        ServerSocket sock = new ServerSocket(port1);
        
//        Socket socketD = new Socket(ipD,port2);
//        Socket socketH = new Socket(ipH,port3);
        
        
        while(true){
        	Socket socket = sock.accept();
		BufferedReader inD = null;
		BufferedWriter outD = null;
		
		
		BufferedWriter outH = null;
        
        //Socket�κ��� InputStream�� ���
        InputStream is = socket.getInputStream();
        //����� InputStream�� ObjectInputStream������� �����
        final ObjectInputStream ois = new ObjectInputStream(is);
        
        //���۵� byte[] �����͸� ����
        byte[] data = (byte[])ois.readObject();
        
        //�̹��� �����
        ByteArrayInputStream btis = new ByteArrayInputStream(data);
        BufferedImage bufferedImage =ImageIO.read(btis); 
        ImageIO.write(bufferedImage, "jpg", new File(path));

//		inD = new BufferedReader(new InputStreamReader(socketD.getInputStream()));
//		outD = new BufferedWriter(new OutputStreamWriter(socketD.getOutputStream()));
//      outH = new BufferedWriter(new OutputStreamWriter(socketH.getOutputStream()));
	
//		outD.write("Done");//�������� �Ϸ� �޼���
//		outD.flush();
        
//		result = inD.readLine();//������� ����
//        outH.write(result);//������� ��ƽ ������ ����
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
