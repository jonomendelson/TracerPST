var canvas = document.getElementById("myCanvas");
var ctx = canvas.getContext("2d");

var TREE_HEIGHT = 1000;

var TREE_SHIFT = 0;

var TREE_X = 1000+TREE_SHIFT;
var TREE_Y = 750;

var frame = 0;

var images = [];
var loadFrame = 0;

var spacings = [{id: -1, y_pos: TREE_Y, spacing:128, parent: -10}]; //id, y position, spacing, parent

var SPEED_THRESHOLD = 3;

var future_parents = [];

var drawText = true;

for(var i = 0; i < tree.length; i++){
	future_parents.push(tree[i][1]);
}

var createImage = function(src, title) {
  var img   = new Image();
  img.src   = src;
  img.alt   = title;
  img.title = title;
  return img; 
};

function isParent(obj){
	for(var i = 0; i < future_parents.length; i++){
		if(future_parents[i] == obj){
			return true;
		}
	}
	return false;
}

document.onkeydown = checkKey;

function checkKey(e) {
    e = e || window.event;

    if (e.keyCode == '38') {
        // up arrow
    }
    else if (e.keyCode == '40') {
        // down arrow
    }
    else if (e.keyCode == '65') {
		frame--; drawFrame(); drawTree();
    }
    else if (e.keyCode == '68') {
     	frame++; drawFrame(); drawTree();
    }

}

function loadTo(frame, stop){
	if(frame < stop){
		load(frame+1);
		if(frame%50 == 0){console.log(frame);}
		window.setTimeout(function(){loadTo(frame+1, stop);}, 2);
	}
}

function load(i){
	var filename = "../Datasets/FinalCroppedFrames/" + (i) + ".jpg";
	images.push(createImage(filename, "unimp"));
}

function leftPad(number, targetLength) {
    var output = number + '';
    while (output.length < targetLength) {
        output = '0' + output;
    }
    return output;
}

function isAlive(speed, intensity, area){
	var prob = 0.044285*speed+0.001866*intensity+0.003407*area-0.6;
	if(prob > 0.35){
		return true;
	}
	return false;
}

function drawFrame(){
	ctx.clearRect(0, 0, 600, canvas.height);
	ctx.drawImage(images[frame],5,-8);
 	for(var i = 0; i < data[frame].length; i++){
		if(isAlive(data[frame][i][5], data[frame][i][6], data[frame][i][4])){
			ctx.strokeStyle="#0000FF";
		}else{
			ctx.strokeStyle="#FF0000";
		}

		ctx.beginPath();
		ctx.arc(data[frame][i][1]+5, data[frame][i][2]-5, Math.sqrt(data[frame][i][4]/Math.PI), 0, 2*Math.PI);
		ctx.closePath();
		ctx.stroke();

		ctx.font = "20pt Arial";
		ctx.fillStyle = "#FF0000";
		if(drawText){
			ctx.fillText(data[frame][i][0],data[frame][i][1],data[frame][i][2]);
		}
	}
	ctx.fillStyle = "#000000";
	ctx.font = "48pt Arial";
	ctx.fillText(frame, 505, 80);
}

function drawSpeed(){
	ctx.fillStyle="#000000";
	for(var i = 0; i < data[frame].length; i++){
		if(data[frame][i][0] != -1){
			ctx.fillRect(TREE_X-TREE_SHIFT+frame/1.5, 100+data[frame][i][0]*50-data[frame][i][5], 1, 1);
		}
	}
}
 
function drawSplitConf(){
	ctx.clearRect(0, 0, 600, canvas.height);
	ctx.drawImage(images[frame],5,-8);
 	for(var i = 0; i < data[frame].length; i++){
		ctx.font="9pt Arial";
		ctx.fillStyle="#FF0000";
		ctx.fillText(Math.round(data[frame][i][4]*100)/100,data[frame][i][1],data[frame][i][2]);
	}
}

function drawGrid(){
	console.log("Drawing grid...");
	ctx.fillStyle="#000000";	
	for(var i = 0; i < 12; i++){
		ctx.beginPath();
		ctx.moveTo(TREE_X+i*(100/1.5), 0);
		ctx.lineTo(TREE_X+i*(100/1.5), 4800);
		ctx.stroke();
	}
}

function drawTree(){
	console.log(frame);
	var objsToDraw = 0;
	ctx.fillStyle="#000000";	
	ctx.font="10pt Arial";
	for(var obj = 0; obj < tree.length; obj++){
 		if(tree[obj][3] <= frame){ //[cellid, parentid, lifetime, birthtime]
			if((tree[obj][2]+tree[obj][3]) >= frame){
				objsToDraw++;		
				
				//check if id has a spacing set already
				var drawX = TREE_X+frame/1.5;
				var drawY = -1;
				var currSpacing = -1;
				for(var i = 0; i < spacings.length; i++){
					if(tree[obj][0] == spacings[i].id){
						drawY = spacings[i].y_pos;	
						currSpacing = spacings[i].spacing;
					}
				}
				if(drawY == -1){ //check if obj to be drawn has a parent that was already drawn
					for(var i = 0; i < spacings.length; i++){
						if(tree[obj][1] == spacings[i].id){
							currSpacing = spacings[i].spacing/2;
							drawY = spacings[i].y_pos-currSpacing/2;
							i = spacings.length; //break
						}
					}
					ctx.beginPath();
					ctx.moveTo(drawX,drawY);
					for(var i = 0; i < spacings.length; i++){
						if(spacings[i].parent == tree[obj][1]){ //look for other children. We must be above them.
							drawY = Math.max(drawY, spacings[i].y_pos+currSpacing);
							if(spacings[i].parent != -1){
								ctx.lineTo(drawX,drawY);
							}
						}
					}
					ctx.stroke();
					ctx.fillText(tree[obj][0], drawX, drawY);
					spacings.push({id:tree[obj][0], parent:tree[obj][1], spacing:currSpacing, y_pos:drawY});
				}

			ctx.fillRect(drawX, drawY, 2, 2);			
			}
		}
	}
}

function findObj(id){
	var result = [];
	for(var i = 0; i < data[frame].length; i++){
		if(data[frame][i][0] == id){
			result.push(data[frame][i]);
		}
	}
	return result;
}
