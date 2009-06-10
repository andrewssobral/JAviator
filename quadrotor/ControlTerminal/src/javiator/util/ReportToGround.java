package javiator.util;

import javiator.util.ControllerConstants;
import javiator.util.ReportToGround;
import javiator.util.SensorData;
import javiator.util.MotorSignals;
import javiator.util.MotorOffsets;
import javiator.util.StateAndMode;
import javiator.util.Packet;

/**
 * Represents data sent to the ground station ("terminal") by the control.
 */
public class ReportToGround extends NumeratedSendable
{
	public final SensorData   sensorData;
    public final MotorSignals motorSignals;
    public final MotorOffsets motorOffsets;
    public final StateAndMode stateAndMode;

    public final static int PACKET_SIZE =
	    SensorData   .PACKET_SIZE +
	    MotorSignals .PACKET_SIZE +
	    MotorOffsets .PACKET_SIZE +
	    StateAndMode .PACKET_SIZE;
  
    public ReportToGround( )
    {
	    sensorData   = new SensorData( );
	    motorSignals = new MotorSignals( );
	    motorOffsets = new MotorOffsets( );
	    stateAndMode = new StateAndMode(
            (byte) 0, (byte) ControllerConstants.ALT_MODE_GROUND );
    }
  
    public ReportToGround(
    		SensorData   sensorData,
    		MotorSignals motorSignals, 
    		MotorOffsets motorOffsets,
    		byte         controlState,
    		byte         altitudeMode )
	{
		this.sensorData   = (SensorData) sensorData.clone( );
		this.motorSignals = (MotorSignals) motorSignals.clone( );
		this.motorOffsets = (MotorOffsets) motorOffsets.clone( );
		this.stateAndMode = new StateAndMode( controlState, altitudeMode );
	}

	public ReportToGround deepClone( )
    { 
	    ReportToGround clone = new ReportToGround( );
	    copyTo( clone );

	    return( clone );
    }
	
	public Object clone( )
	{
		return deepClone( );
	}

	public void copyTo( Copyable to )
	{
		ReportToGround copy = (ReportToGround) to;
	    sensorData   .copyTo( copy.sensorData );
	    motorSignals .copyTo( copy.motorSignals );
	    motorOffsets .copyTo( copy.motorOffsets );
	    stateAndMode .copyTo( copy.stateAndMode );
	}

	public void fromPacket( Packet packet )
	{
        int offset = 0;
		sensorData.decode( packet, offset );
		offset += SensorData.PACKET_SIZE;
		motorSignals.decode( packet, offset );
		offset += MotorSignals.PACKET_SIZE;
		motorOffsets.decode( packet, offset );
		offset += MotorOffsets.PACKET_SIZE;
		stateAndMode.decode( packet, offset );
	}

	public void reset( )
	{
		sensorData   .reset( );
		motorSignals .reset( );
		motorOffsets .reset( );
		stateAndMode .reset( );
	}

	public Packet toPacket( )
	{
		Packet packet = new Packet(
			SensorData   .PACKET_SIZE +
			MotorSignals .PACKET_SIZE + 
			MotorOffsets .PACKET_SIZE +
			StateAndMode .PACKET_SIZE );
		encode( packet, 0 );

		return( packet );
	}

	public void encode( Packet packet, int offset )
	{
		sensorData.encode( packet, offset );
		offset += SensorData.PACKET_SIZE;
		motorSignals.encode( packet, offset );
		offset += MotorSignals.PACKET_SIZE;
		motorOffsets.encode( packet, offset );
		offset += MotorOffsets.PACKET_SIZE;
		stateAndMode.encode( packet, offset );
	}

	public Packet toPacket( byte type )
	{
		Packet ans = toPacket( );
		ans.type = type;
		ans.calcChecksum( );

		return( ans );
	}
}
