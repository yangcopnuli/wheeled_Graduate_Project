/***
 * @CreatedTime   2022-04-23 19:57:04
 * @LastEditors   未定义
 * @LastEditTime  2022-04-25 15:29:11
 * @FilePath      \bishe\Robot.h
 */

#ifndef ROBOT_H
#define ROBOT_H

#include <webots/Robot.hpp>
#include <webots/Motor.hpp>
#include <webots/Keyboard.hpp>
#include <webots/InertialUnit.hpp>
#include <webots/Gyro.hpp>
#include <webots/Camera.hpp>
#include <webots/GPS.hpp>
#include <webots/PositionSensor.hpp>
#include <iostream>
#include <fstream>
#include <math.h>
#include <Eigen>
#include "MyDefine.h"
#include "Pid.h"
#include "Leg.h"

using namespace Eigen;
using namespace webots;
using namespace std;

class MyRobot : public Robot
{
private:
    /* data */
    u8 time_step;        // 毫秒
    float time;          // 秒
    float sampling_time; // 开始检测的时间,秒
    float yaw_get;

    Camera *camera;
    Gyro *gyro;
    InertialUnit *imu;
    GPS *gps;
    PositionSensor *encoder_wheelL, *encoder_wheelR, *encoder_BL, *encoder_BR, *encoder_FL, *encoder_FR;
    Motor *BL_legmotor, *BR_legmotor, *FL_legmotor, *FR_legmotor, *L_Wheelmotor, *R_Wheelmotor;
    Keyboard *mkeyboard;

    PID_Controller turn_pid;
    PID_Controller split_pid;
    PID_Controller roll_pid;
    Matrix<float, 12, 4> K_coeff;

    float acc_up_max, acc_down_max, acc_now;

    u8 stop_flag;

public:
    MyRobot();
    ~MyRobot();

    u8 sampling_flag;

    LegClass leg_L, leg_R, leg_simplified;
    DataStructure velocity, yaw, pitch, roll;

    float balance_angle;
    static MyRobot *get()
    {
        static MyRobot robot;
        return &robot;
    }

    u8 jump(float t_clk, float *leg_L, float *leg_R);
    void status_update(LegClass *leg_sim, LegClass *leg_L, LegClass *leg_R,
                       DataStructure pitch, DataStructure roll, DataStructure yaw,
                       float dt, float v_set);
    void MyStep();
    void Wait(int ms);
    void run();
    float limitVelocity(float speed_set, float L0);
    double getVelNow() { return velocity.now; };
    double getVelSet() { return velocity.set; };
    double getWheelLeftTorque() { return L_Wheelmotor->getTorqueFeedback(); };
    double getWheelRightTorque() { return R_Wheelmotor->getTorqueFeedback(); };
};

#endif
