var assert = require('assert');
var binding = require('./build/Release/binding');

binding.regProfiler();
binding.unregProfiler();

function loop () {
	while(true){
		//
	}
}

(function zzz(){
	loop();
}
)();


// console.log('binding.tryProfile() =', binding.tryProfile());
