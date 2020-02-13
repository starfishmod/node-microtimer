"use strict"; 

const worker = require("streaming-worker");
const through = require('through');
const path = require("path");

const addon_path = path.join(__dirname, "build/Release/micro_timer");


module.exports = function(period,cb){
	var period = period;
	var cb = cb;
	
	const mt = worker(addon_path, {period: period});
	mt.from.on('period',cb);
	
	return {
		close:function(){
			mt.close()
		}
		
	}
}

/*function now(hr){
		var hr = hr||process.hrtime();
		return (hr[0] * 1e9 + hr[1] );/// 1e3;
		//return (hr[0] * 1e9 + hr[1] )/ 1e6;
	}
	
	var lastTime = process.hrtime();
	var lastClockTime = process.hrtime();

// Option 1 - Just use the emitter interface
wobbly_sensor.from.on('period', function(sample){
	var tDiff = now(process.hrtime(lastTime));

	console.log(tDiff);
	lastTime=process.hrtime();

});


      
// The addon must poll for the close signal (see sensor_sim.cpp)
setTimeout(function(){wobbly_sensor.close()}, 50000);*/
