package com.example.irrigaflor

import android.content.Context

class SecurityPreferences(context: Context) {
    private val mSharedPreferences = context.getSharedPreferences("horario", Context.MODE_PRIVATE)
    fun storeInt(key:String, value: Int){
        mSharedPreferences.edit().putInt(key, value).apply()
    }
    fun getInt(key: String): Int{
        return mSharedPreferences.getInt(key, 0)

    }

}