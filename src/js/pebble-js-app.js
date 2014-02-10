function findProgramSettings(program, week, session) {
	console.log("findProgramSettings: program = " + program + ", week = " + week + ", session = " + session);

	$.getJSON("resources/programs.json", function(json) {
		console.log(json);
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
			var config = JSON.parse(e.response);
			console.log("Configuration window returned: " + JSON.stringify(config));

			findProgramSettings(config.program, config.week, config.session);
		} else {
			console.log("no response from configuration");
		}

	}
);