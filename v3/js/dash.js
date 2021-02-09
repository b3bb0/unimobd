
if (!Object.prototype.forEach) {
	Object.defineProperty(Object.prototype, 'forEach', {
		value: function (callback, thisArg) {
			if (this == null) {
				throw new TypeError('Not an object');
			}
			thisArg = thisArg || window;
			for (var key in this) {
				if (this.hasOwnProperty(key)) {
					callback.call(thisArg, this[key], key, this);
				}
			}
		}
	});
}

document.fonts && document.fonts.forEach(function(font) {
    font.loaded.then(function() {
        if (font.family.match(/Led/)) {
            document.gauges.forEach(function(gauge) {
                gauge.update();
                gauge.options.renderTo.style.visibility = 'visible';
            });
        }
    });
});

/*
var timers = [];

function animateGauges() {
    document.gauges.forEach(function(gauge) {
        timers.push(setInterval(function() {
            gauge.value = Math.random() *
                (gauge.options.maxValue - gauge.options.minValue) +
                gauge.options.minValue;
        }, gauge.animation.duration + 50));
    });
}

function stopGaugesAnimation() {
    timers.forEach(function(timer) {
        clearInterval(timer);
    });
}

function resize() {
    var size = parseFloat(document.getElementById('gauge-size').value) || 400;

    document.gauges.forEach(function (gauge) {
        gauge.update({ width: size, height: size });
    });
}

function setText() {
    var text = document.getElementById('gauge-text').value;

    document.gauges.forEach(function (gauge) {
        gauge.update({ valueText: text });
    });
}



$( document ).ready(function() {
    $('.clickable').click(function() {
        console.log('clicked!')
    })
});
*/