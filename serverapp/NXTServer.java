/**
 * NXTServer.java Programme de gestion des mouvements du robot, 
 *                il se doit d'être integré dans le module NXT
 *                et tourner sous lejOS
 *
 * Copyright 2009 Muhiy-eddine Cherik <muhiy-eddine.cherik@heig-vd.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;

import lejos.nxt.Battery;
import lejos.nxt.LCD;
import lejos.nxt.Motor;
import lejos.nxt.SensorPort;
import lejos.nxt.UltrasonicSensor;
import lejos.nxt.comm.USB;
import lejos.nxt.comm.USBConnection;

public class NXTServer {
	
	// map ID command with each behaviour
	public static void execCommand(byte cmd){
		switch(cmd){	
			case GO:
				Motor.A.forward();
				Motor.B.forward();
				break;
			
			case BACK:
				Motor.A.backward();
				Motor.B.backward();
				break;
			
			case RIGHT:
				Motor.A.stop();
				Motor.B.stop();
				Motor.A.rotate(45);
				break;
			
			case LEFT:
				Motor.A.stop();
				Motor.B.stop();
				Motor.B.rotate(45);
				break;
			
			case STOP:
				Motor.A.stop();
				Motor.B.stop();
		}
		
	}
	
	NXTServer(){	
		// get USB connection ready
        LCD.drawString("Connecting...", 0, 1);
	    USBConnection usbStream = USB.waitForConnection(); 
			
		// open stream
		dataIn = usbStream.openDataInputStream();
		dataOut= usbStream.openDataOutputStream();
	
		LCD.drawString("OK", 14, 1);
		
		sensor[0] = new UltrasonicSensor(SensorPort.S1);
        sensor[1] = new UltrasonicSensor(SensorPort.S4);
        
        distance = new int[2];
	}
	
	public void launch() {

		while(true){	   
			
			try {
				// get command from user
				if(dataIn.available() > 0)
					cmd = dataIn.readByte();
				
				// get sensors data
				distance[0] = (byte) sensor[0].getDistance();
				distance[1] = (byte) sensor[1].getDistance();
				
				// get battery voltage
				voltage = Battery.getVoltage();
				
				// is an obstacle is near the robot
				if((distance[0] != -1 && distance[0] <= MINDISTANCE && cmd == BACK) ||
				   (distance[1] != -1 && distance[1] <= MINDISTANCE && cmd == GO)){
					cmd   = STOP;
					state = HIT;  // send hit code to client app
				} 
				else if(voltage <= MINVOLTAGE){
					cmd   = STOP;
					state = LOWBATT; // send battery low code
				}
				else
					state = NORMAL;  // otherwise -> normal state

				// send state code
				dataOut.writeByte(state);
				
				// execute order
				execCommand(cmd);
			
				LCD.drawString("Command received!", 0, 3);			
			
			}catch (IOException e){
				System.out.println(" read error "+e); 
			}	
			
		    // Display NXT informations 
		    LCD.drawInt(distance[0], 1, 5);
		    LCD.drawInt(distance[1], 10, 5);
		    LCD.drawString("Battery: " + voltage + " mV", 0, 7);
	        
		    LCD.refresh();		
		
		}

	}
	
	public static void main(String args[]){
		NXTServer mvt = new NXTServer();
		mvt.launch();
		
		LCD.drawString("Disconnect..", 0, 3);
		USB.waitForDisconnect(10);	// wait remote for disconnection
	}
	
	private int   distance[];
	private int   state   = NORMAL;
	
	private byte  cmd     = STOP;
	private float voltage = 0;
	
	// ultrasonic sensors
	private UltrasonicSensor sensor[] = new UltrasonicSensor[2];
	
	// USB communication streams
	private DataInputStream  dataIn;
	private DataOutputStream dataOut;
	
	// commands
	public static final byte STOP  = 0;
	public static final byte GO    = 1;
	public static final byte BACK  = 2;
	public static final byte LEFT  = 3;
	public static final byte RIGHT = 4;
	
	// robot states code
	public static final int NORMAL  = 9;
	public static final int LOWBATT = 73;
	public static final int HIT     = 89;
	
	public static final int MINDISTANCE = 15;   // in cm
	public static final float MINVOLTAGE  = (float)6.5; // in V
}

