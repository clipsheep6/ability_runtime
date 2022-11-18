package com.huawei.ci.common.utils;

import com.huawei.ci.common.enums.DateEnum;

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;

public class DateUtil {
    /**
     * 获取当前时间的日期
     *
     * @return 当前日期string
     */
    public static String getCurrentDate(){
        SimpleDateFormat dateFormat = DateEnum.DAY.getFormat();
        return dateFormat.format(new Date());
    }

    /**
     * 获取七天前的日期
     */
    public static String getBeforeSevenDayString(){
        SimpleDateFormat dateFormat = DateEnum.DAY.getFormat();
        Calendar calendar = Calendar.getInstance();
        calendar.add(Calendar.DATE, -7);
        return dateFormat.format(calendar.getTime());
    }

    /**
     * 获取当前天在这一周的第几天
     *
     * @return int 第几天
     */
    public static int getDayNumberOfWeek(){
        Calendar calendar = Calendar.getInstance();
        calendar.setTime(new Date());
        return calendar.get(Calendar.DAY_OF_WEEK);
    }
}
