
/***
 * @CreatedTime   2022-04-23 19:57:04
 * @LastEditors   未定义
 * @LastEditTime  2022-04-25 15:18:25
 * @FilePath      \bishe\Robot.cpp
 */

#include "Robot.h"

MyRobot::MyRobot() : balance_angle(-0.0064)
{
    time_step = getBasicTimeStep();
    mkeyboard = getKeyboard(), mkeyboard->enable(time_step);
    camera = getCamera("camera"), camera->enable(time_step);
    gyro = getGyro("gyro"), gyro->enable(time_step);
    imu = getInertialUnit("imu"), imu->enable(time_step);
    gps = getGPS("gps"), gps->enable(time_step);
    encoder_wheelL = getPositionSensor("encoder_wheelL"), encoder_wheelL->enable(time_step);
    encoder_wheelR = getPositionSensor("encoder_wheelR"), encoder_wheelR->enable(time_step);
    encoder_BL = getPositionSensor("encoder_BL"), encoder_BL->enable(time_step);
    encoder_BR = getPositionSensor("encoder_BR"), encoder_BR->enable(time_step);
    encoder_FL = getPositionSensor("encoder_FL"), encoder_FL->enable(time_step);
    encoder_FR = getPositionSensor("encoder_FR"), encoder_FR->enable(time_step);
    L_Wheelmotor = getMotor("L_Motor"), BL_legmotor = getMotor("BL_Motor"), FL_legmotor = getMotor("FL_Motor");
    R_Wheelmotor = getMotor("R_Motor"), BR_legmotor = getMotor("BR_Motor"), FR_legmotor = getMotor("FR_Motor");
    L_Wheelmotor->setVelocity(0), R_Wheelmotor->setVelocity(0);
    L_Wheelmotor->setPosition(INFINITY), R_Wheelmotor->setPosition(INFINITY);
    BL_legmotor->setPosition(0), BR_legmotor->setPosition(0), FL_legmotor->setPosition(0), FR_legmotor->setPosition(0);

    BL_legmotor->enableTorqueFeedback(time_step), BR_legmotor->enableTorqueFeedback(time_step), FL_legmotor->enableTorqueFeedback(time_step), FR_legmotor->enableTorqueFeedback(time_step);
    L_Wheelmotor->enableTorqueFeedback(time_step), R_Wheelmotor->enableTorqueFeedback(time_step);
    roll_set = 0, yaw_set = 0;
    // 调参
    velocity_set = 0;
    turn_pid.update(1.0, 0.0, 0.01, 0);
    split_pid.update(1.0, 0.0, 0.01, 0);
    roll_pid.update(10.0, 0.0, 0.1, 0);

    K_coeff << -25.1567889744640, 61.2559605691941, -112.327246113332, -35.6263347175684,
        -83.9861911112494, 121.140685145816, -65.9771359435778, 17.2481951925006,
        33.6610421184600, -42.6418410652893, -11.0317309274503, -4.80018523419281,
        -9.05539120435989, 13.1160333999245, -7.01674273659039, 2.27854529744476,
        -2.42963819944445, 2.96583756258825, -1.27873684098999, -9.79727553372923,
        -33.3889111556499, 45.4503947942905, -23.1038779169637, 5.01984122990087,
        15.2605231872654, -17.3233935829798, 3.52444935584224, -17.1192201057835,
        -56.8482058701575, 76.5874821548405, -38.4309371394415, 8.28309798187163,
        -105.585007861340, 143.726768120002, -73.0608770053566, 15.8741317794817,
        30.7327623567397, -37.5152074289416, 16.1748837685857, 123.926822204711,
        -13.4507979321613, 18.2214231419328, -9.37238207023173, 2.19134083488087,
        4.38435529910105, -5.39904210845876, 2.36478804435298, 11.1226669194140;
}

MyRobot::~MyRobot()
{
}

void MyRobot::MyStep()
{
    if (step(time_step) == -1)
        exit(EXIT_SUCCESS);
}
/**
 * @brief: 毫秒级延时
 * @author: Dandelion
 * @Date: 2023-03-27 16:35:24
 * @param {int} ms
 * @return {*}
 */
void MyRobot::Wait(int ms)
{
    float start_time = getTime();
    float s = ms / 1000.0;
    while (s + start_time >= getTime())
        MyStep();
}
/**
 * @brief:
 * @author: Dandelion
 * @Date: 2023-04-01 00:10:38
 * @param {LegClass} *leg_sim
 * @param {LegClass} *leg_L
 * @param {LegClass} *leg_R
 * @param {float} pitch
 * @param {float} pitch_dot
 * @param {float} dt
 * @param {float} v_set
 * @return {*}
 */
void MyRobot::status_update(LegClass *leg_sim, LegClass *leg_L, LegClass *leg_R,
                            float pitch, float pitch_dot, float dt, float v_set)
{
    leg_L->F_set = 13.12 / 2 * 9.81;
    leg_R->F_set = 13.12 / 2 * 9.81;
    // 获取当前机器人状态信息
    leg_L->dis = encoder_wheelL->getValue() * 0.05;
    leg_R->dis = encoder_wheelR->getValue() * 0.05;
    leg_sim->dis = (leg_L->dis + leg_R->dis) / 2.0;

    leg_L->dis_dot = (leg_L->dis - leg_R->dis_last) * 1000.f / time_step;
    leg_R->dis_dot = (leg_R->dis - leg_R->dis_last) * 1000.f / time_step;
    leg_sim->dis_dot = (leg_L->dis_dot + leg_R->dis_dot) / 2.0;

    leg_L->dis_last = leg_L->dis;
    leg_R->dis_last = leg_R->dis;
    leg_sim->dis_last = (leg_L->dis_last + leg_R->dis_last) / 2;

    leg_L->TL_now = BL_legmotor->getTorqueFeedback();
    leg_L->TR_now = FL_legmotor->getTorqueFeedback();
    leg_R->TL_now = BR_legmotor->getTorqueFeedback();
    leg_R->TR_now = FR_legmotor->getTorqueFeedback();
    leg_L->TWheel_now = L_Wheelmotor->getTorqueFeedback();
    leg_R->TWheel_now = L_Wheelmotor->getTorqueFeedback();
    // 角度更新，统一从右视图看吧
    leg_L->angle1 = 2.0 / 3.0 * PI - encoder_FL->getValue();
    leg_L->angle4 = 1.0 / 3.0 * PI + encoder_BL->getValue();
    cout << "Left_ang1 " << leg_L->angle1 << " Left_ang4 " << leg_L->angle4 << " L0 " << leg_L->L0_now << " angle0 " << leg_L->angle0 << " xc " << leg_L->xc << " yc " << leg_L->yc << endl;
    leg_L->Zjie(leg_L->angle1, leg_L->angle4, pitch);
    leg_R->angle1 = 2.0 / 3.0 * PI - encoder_FR->getValue();
    leg_R->angle4 = 1.0 / 3.0 * PI + encoder_BR->getValue();
    cout << "Right_ang1 " << leg_R->angle1 << " Right_ang4 " << leg_R->angle4 << " L0 " << leg_R->L0_now << " angle0 " << leg_R->angle0 << " xc " << leg_R->xc << " yc " << leg_R->yc << endl;
    leg_R->Zjie(leg_R->angle1, leg_R->angle4, pitch);
    // 计算K矩阵数据，根据l0_now拟合得到
    static Matrix<float, 2, 1> u;
    static Matrix<float, 2, 1> Torque;
    leg_sim->angle1 = (leg_L->angle1 + leg_R->angle1) / 2;
    leg_sim->angle4 = (leg_R->angle4 + leg_R->angle4) / 2;
    cout << "sim_angle1: " << leg_sim->angle1 << " sim_angle4: " << leg_sim->angle4 << endl;
    float angle0_before = leg_sim->angle0;
    leg_sim->Zjie(leg_sim->angle1, leg_sim->angle4, pitch);
    leg_sim->angle0_dot = (leg_sim->angle0 - angle0_before) / dt;
    // 限幅
    // pitch_dot = Deadzone(pitch_dot, 0.01);
    // leg_sim->dis_dot = Deadzone(leg_sim->dis_dot, 0.01);
    // leg_sim->angle0_dot = Deadzone(leg_sim->angle0_dot, 0.1);
    printf("K: ");
    for (size_t col = 0; col < 6; col++)
    {
        /* code */
        for (size_t row = 0; row < 2; row++)
        {
            /* code */
            int num = col * 2 + row;
            leg_sim->K(row, col) = K_coeff(num, 0) * pow(leg_sim->L0_now, 3) +
                                   K_coeff(num, 1) * pow(leg_sim->L0_now, 2) +
                                   K_coeff(num, 2) * leg_sim->L0_now +
                                   K_coeff(num, 3);
            printf("%f ", leg_sim->K(row, col));
        }
    }
    printf("\n");
    // 状态更新
    leg_sim->dis_desire += v_set * dt;
    printf("Xd:%f\n", leg_sim->dis_desire);
    leg_sim->Xd << 0, 0, leg_sim->dis_desire, 0, 0, 0;
    leg_sim->X << leg_sim->angle0, leg_sim->angle0_dot, leg_sim->dis, leg_sim->dis_dot, pitch, pitch_dot;
    cout << "ang0 " << leg_sim->angle0 << " ang_past " << angle0_before << " ang0_dot " << leg_sim->angle0_dot << " dis " << leg_sim->dis << " dis_dot " << leg_sim->dis_dot << " pitch " << pitch << " pitch_dot " << pitch_dot << endl;
    u = leg_sim->K * (leg_sim->Xd - leg_sim->X); // u = K(Xd-X);
    leg_sim->TWheel_set = u(0, 0);
    leg_sim->Tp_set = u(1, 0);
    cout << "Tp_set " << leg_sim->Tp_set << " Twheel_set " << leg_sim->TWheel_set << endl;
    leg_L->TWheel_set = leg_sim->TWheel_set / 2.0;
    leg_R->TWheel_set = leg_sim->TWheel_set / 2.0;

    leg_L->Tp_set = leg_sim->Tp_set / 2.0;
    leg_R->Tp_set = leg_sim->Tp_set / 2.0;
    // pid补偿
    float out_L, out_R, out_roll, out_spilt, out_turn;
    out_L = leg_L->supportF_pid.compute(leg_L->L0_set, 0, leg_L->L0_now, leg_L->L0_dot, dt);
    out_R = leg_R->supportF_pid.compute(leg_R->L0_set, 0, leg_R->L0_now, leg_R->L0_dot, dt);
    out_roll = roll_pid.compute(roll_set, 0, roll, roll_dot, dt);
    out_spilt = split_pid.compute(0, 0, leg_L->angle0 - leg_R->angle0, leg_L->angle0_dot - leg_R->angle0_dot, dt);
    out_turn = turn_pid.compute(yaw_set, 0, yaw, yaw_dot, dt);
    // 左腿VMC解算
    leg_L->L0_dot = (leg_L->L0_now - leg_L->L0_last) / dt;
    leg_L->L0_last = leg_L->L0_now;
    leg_L->F_set += out_L;
    leg_L->F_set += out_roll;
    leg_L->Tp_set += out_spilt; // 这里的正负号没研究过
    cout << "LF_set: " << leg_L->F_set << " pid " << out_L << " ang1 " << leg_L->angle1 << " ang2 " << leg_L->angle2 << " ang3 " << leg_L->angle3 << " ang4 " << leg_L->angle4 << endl;
    Torque = leg_L->VMC(leg_L->F_set, leg_L->Tp_set);
    leg_L->TL_set = -Torque(0, 0);
    leg_L->TR_set = Torque(1, 0);
    // 右腿VMC解算
    leg_R->L0_dot = (leg_R->L0_now - leg_R->L0_last) / dt;
    leg_R->L0_last = leg_R->L0_now;
    leg_R->F_set += out_R;
    leg_R->F_set -= out_roll;
    leg_R->Tp_set -= out_spilt;
    cout << "RF_set: " << leg_R->F_set << " pid " << out_R << " ang1 " << leg_R->angle1 << " ang2 " << leg_R->angle2 << " ang3 " << leg_R->angle3 << " ang4 " << leg_R->angle4 << endl;
    Torque = leg_R->VMC(leg_R->F_set, leg_R->Tp_set);
    leg_R->TL_set = -Torque(0, 0);
    leg_R->TR_set = Torque(1, 0);
}

void MyRobot::run()
{
    static int last_key;

    static PID_Controller vertical_pid(9, 0, 1, 60);
    static PID_Controller velocity_pid(0.13, 0.003, 0.02, 1.0);
    static PID_Controller turn_pid(12, 0, 0.3, 0);
    static PID_Controller roll_pid(0.18, 0, 0.02, 0);

    pitch = imu->getRollPitchYaw()[1];
    pitch_dot = gyro->getValues()[2];
    roll = imu->getRollPitchYaw()[0];
    roll_dot = gyro->getValues()[0];
    yaw_get = imu->getRollPitchYaw()[2];
    yaw_dot = gyro->getValues()[1];
    float robot_x = gps->getValues()[0];

    if (yaw_get - yaw_get_last > 1.5 * PI)
        yaw += yaw_get - yaw_get_last - 2 * PI;
    else if (yaw_get - yaw_get_last < -1.5 * PI)
        yaw += yaw_get - yaw_get_last + 2 * PI;
    else
        yaw += yaw_get - yaw_get_last;

    yaw_get_last = yaw_get;
    if (time == 0)
        yaw_set = yaw;

    // 时序更新
    time = getTime();

    int key = mkeyboard->getKey();
    while (key > 0)
    {
        printf("key: %d\n", key);

        Keyboard kboard;
        switch (key)
        {
        case kboard.UP:
            velocity_set += 0.02f;
            break;
        case kboard.DOWN:
            velocity_set -= 0.02f;
            break;
        case kboard.RIGHT:
            if (ABS(velocity_set) > 0.1)
                yaw_set = yaw - 0.1f;
            else
                yaw_set = yaw - 0.2f;
            break;
        case kboard.LEFT:
            if (ABS(velocity_set) > 0.1)
                yaw_set = yaw + 0.1f;
            else
                yaw_set = yaw + 0.2f;
            break;
        case kboard.END:
            velocity_set = 0;
            break;
        case 'W':
            leg_L.yc += 0.002f;
            leg_R.yc += 0.002f;
            break;
        case 'S':
            leg_L.yc -= 0.002f;
            leg_R.yc -= 0.002f;
            break;
        case 'A':
            roll_set -= 0.02f;
            break;
        case 'D':
            roll_set += 0.02f;
            break;
        case ' ':
            if (last_key != key)
                stop_flag = True;
            break;
        }
        last_key = key;
        key = mkeyboard->getKey();
    }

    velocity_set = Limit(velocity_set, 3, -3);

    cout << "虚拟腿更新：" << endl;
    status_update(&leg_simplified, &leg_L, &leg_R, pitch - balance_angle, pitch_dot, time_step / 1000.f, velocity_set);

    cout << "L_TL " << leg_L.TL_now << " L_TR " << leg_L.TR_now << " R_TL " << leg_R.TL_now << " R_TR " << leg_R.TR_now << " L_Wheel " << leg_L.TWheel_now << " R_Wheel " << leg_R.TWheel_now << endl;
    static int tick = 500;
    if (tick == 0)
    {
        /* code */
        BL_legmotor->setTorque(leg_L.TL_set);
        FL_legmotor->setTorque(leg_L.TR_set);
        BR_legmotor->setTorque(leg_R.TL_set);
        FR_legmotor->setTorque(leg_R.TR_set);
    }
    else
    {
        tick--;
    }

    L_Wheelmotor->setTorque(leg_L.TWheel_set);
    R_Wheelmotor->setTorque(leg_R.TWheel_set);

    cout << "yaw " << this->yaw << " roll " << this->roll << " pitch " << this->pitch << endl;

    printf("dt:%d, BackLeft:%f, FrontLeft:%f, BackRight:%f, FrontRight:%f, WheelL:%f, WheelR:%f\n",
           time_step, leg_L.TL_set, leg_L.TR_set, leg_R.TL_set, leg_R.TR_set, leg_L.TWheel_set, leg_R.TWheel_set);
    // ofstream outfile;
    // outfile.open("data2.dat", ios::trunc);
    // outfile << time << ' ' << pitch << ' ' << disL_dot << ' ' << robot_x << ' ' << L_Torque << endl;
    // outfile.close();
}
