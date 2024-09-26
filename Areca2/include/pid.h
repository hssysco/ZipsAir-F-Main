#ifndef PID_H
#define PID_H
/*-------------------------------------------------------------*/
/*		Includes and dependencies			*/
/*-------------------------------------------------------------*/
/*-------------------------------------------------------------*/
/*		Macros and definitions				*/
/*-------------------------------------------------------------*/
/*-------------------------------------------------------------*/
/*		Typedefs enums & structs			*/
/*-------------------------------------------------------------*/

/**
 * Defines if the controler is direct or reverse
 */
typedef enum PID_CTRL_DIR  
{
	E_PID_DIRECT,
	E_PID_REVERSE,
}PIDCtrlDirT;

/**
 * Structure that holds PID all the PID controller data, multiple instances are
 * posible using different structures for each controller
 */
typedef struct PID_CTRL 
{
	// Input, output and setpoint
	float * input; //!< Current Process Value
	float * output; //!< Corrective Output from PID Controller
	float * setpoint; //!< Controller Setpoint
	// Tuning parameters
	float Kp; //!< Stores the gain for the Proportional term
	float Ki; //!< Stores the gain for the Integral term
	float Kd; //!< Stores the gain for the Derivative term
	// Output minimum and maximum values
	float omin; //!< Maximum value allowed at the output
	float omax; //!< Minimum value allowed at the output
	// Variables for PID algorithm
	float iterm; //!< Accumulator for integral term
	float lastin; //!< Last input value for differential term
	// Time related
	unsigned int lasttime; //!< Stores the time when the control loop ran last time
	unsigned int sampletime; //!< Defines the PID sample time
	// Operation mode
	unsigned char automode; //!< Defines if the PID controller is enabled or disabled
	PIDCtrlDirT direction;
}PIDCtrlT;

PIDCtrlT *PID_Create(PIDCtrlT *pPID, float* in, float* out, float* set, float kp, float ki, float kd);
int PID_Compute(PIDCtrlT *pPID);
void PID_Tune(PIDCtrlT *pPID, float kp, float ki, float kd);
void PID_Limits(PIDCtrlT *pPID, float min, float max);
void PID_Auto(PIDCtrlT *pPID);
void PID_Manual(PIDCtrlT *pPID);
void PID_Direction(PIDCtrlT *pPID, PIDCtrlDirT dir);

#endif
// End of Header file

