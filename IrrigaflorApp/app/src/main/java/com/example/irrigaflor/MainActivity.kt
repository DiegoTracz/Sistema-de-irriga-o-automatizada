package com.example.irrigaflor

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import android.widget.NumberPicker
import android.widget.SeekBar
import android.widget.Toast
import kotlinx.android.synthetic.main.activity_main.*
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken
import org.eclipse.paho.client.mqttv3.MqttCallbackExtended
import org.eclipse.paho.client.mqttv3.MqttMessage

class MainActivity : AppCompatActivity(), NumberPicker.OnValueChangeListener {

    private lateinit var mSecurityPreferences: SecurityPreferences
    var horaLiga: Int = 0
    var mqttHelper: MqttHelper? = null
    private var valor1: Int? = null
    private var valor2: Int? = null
    private var valor3: Int? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        startMqtt()

        mSecurityPreferences = SecurityPreferences(this)

        numberPicker.minValue = 0
        numberPicker.maxValue = 23
        numberPicker2.minValue = 0
        numberPicker2.maxValue = 59

        numberPicker.setFormatter { i -> String.format("%02d", i) }
        numberPicker2.setFormatter { i -> String.format("%02d", i) }

        numberPicker.value = horaLiga

        numberPicker.wrapSelectorWheel = true

        numberPicker.setOnValueChangedListener { picker, oldVal, newVal ->
            valor1 = newVal
            valor1?.let { mSecurityPreferences.storeInt("valor1", it) }
        }
        numberPicker2.wrapSelectorWheel = true

        numberPicker2.setOnValueChangedListener { picker, oldVal, newVal ->
            valor2 = newVal
            valor2?.let { mSecurityPreferences.storeInt("valor2", it) }
        }

        seekBar.setOnSeekBarChangeListener(object :SeekBar.OnSeekBarChangeListener{
            override fun onProgressChanged(seekBar: SeekBar, progress: Int, fromUser: Boolean) {
                textView3.text = "$progress /60 min"
                valor3 = progress

                valor3?.let { mSecurityPreferences.storeInt("valor3", it) }
                Toast.makeText(applicationContext, progress.toString(), Toast.LENGTH_LONG).show()
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {
                TODO("Not yet implemented")
            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
                TODO("Not yet implemented")
            }

        })

        valor1 = mSecurityPreferences.getInt("valor1")
        valor2 = mSecurityPreferences.getInt("valor2")
        valor3 = mSecurityPreferences.getInt("valor3")

        button.setOnClickListener {
            mqttHelper!!.publish(MQTT_HORA_LIGA, "$valor1")
            mqttHelper!!.publish(MQTT_MINUTO_LIGA, "$valor2")
            mqttHelper!!.publish(MQTT_MINUTO_DESLIGA, "$valor2")
        }
        numberPicker.value = valor1 as Int
        numberPicker2.value = valor2 as Int
        seekBar.progress = valor3 as Int

    }

    private fun startMqtt() {

        mqttHelper = MqttHelper(getApplicationContext())
        mqttHelper!!.mqttAndroidClient.setCallback(object : MqttCallbackExtended {

            override fun connectComplete(b: Boolean, s: String) {
                Log.w("Debug", "Connected")


            }

            override fun connectionLost(throwable: Throwable) {}

            @Throws(Exception::class)
            override fun messageArrived(topic: String, mqttMessage: MqttMessage) {
                Log.w("Debug", mqttMessage.toString())


            }

            override fun deliveryComplete(iMqttDeliveryToken: IMqttDeliveryToken) {}
        })
    }

    override fun onValueChange(picker: NumberPicker?, oldVal: Int, newVal: Int) {
        TODO("Not yet implemented")
    }
    companion object{
        const val MQTT_HORA_LIGA = "horaLiga"
        const val MQTT_MINUTO_LIGA = "minutoLiga"
        const val MQTT_MINUTO_DESLIGA = "minutoDesl"
    }

}