
function saveProgram(warmup, cooldown, intervals) {
	localStorage.setItem("warmup", warmup);
	localStorage.setItem("cooldown", cooldown);

	//generate a string to pass to the pebble containing all the information needed for the intervals
	var intervalString = "";
	for(var i=0; i<intervals.length; i++) {
		var interval = intervals[i];
			intervalString = intervalString + interval.run + "R" + interval.walk + "W" + interval.repeat + "X";
	}

	localStorage.setItem("intervals", intervalString);

	console.log("Saved program: warmup = " + warmup + " cooldown = " + cooldown + " intervalString = " + intervalString);
}

function sendProgramToPebble() {
	var warmup = localStorage.getItem("warmup");
	var cooldown = localStorage.getItem("cooldown");
	var intervalString = localStorage.getItem("intervals");

	console.log("Sending program to pebble: warmup=" + warmup + " cooldown=" + cooldown + " intervalString=" + intervalString);

	Pebble.sendAppMessage({"warmup": warmup, 
							"cooldown": cooldown,
							"intervals": intervalString
						});
}

Pebble.addEventListener("ready",
    function(e) {
        console.log("JavaScript app is ready and running.");
    }
);

Pebble.addEventListener("showConfiguration",
	function(e) {
		console.log("Showing Configuration");
		Pebble.openURL('http://sdelaney.github.io/interbbleTimer/index.html');
	}
);

Pebble.addEventListener("webviewclosed",
	function(e) {
		console.log("Closing configuration window");

		//get the response object
		if (e.response) {
			console.log(e.response);
			var config = JSON.parse(e.response);
			console.log("Configuration window returned: " + JSON.stringify(config));

			//# minutes for warmup
			var warmup = config.warmup;
			//# minutes for cooldown
			var cooldown = config.cooldown;
			//list of objects with format:
				//run = # minutes
				//walk = # minutes
				//repeat = # times to repeat
			var intervals = config.intervals;
			console.log(JSON.stringify(config.intervals));

			saveProgram(warmup, cooldown, intervals);

			sendProgramToPebble();
		} else {
			console.log("no response from configuration");
		}

	}
);