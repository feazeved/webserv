// Function responsible to listen to wasd / arrows and send the respective json to the server SSE
// document.addEventListener('keydown', (e) => {
// 	let dx = 0;
// 	let dy = 0;

// 	if (e.key == 'ArrowUp')
// 		dy = -5;
// 	if (e.key == 'ArrowDown')
// 		dy = 5;
// 	if (e.key == 'ArrowLeft')
// 		dx = -5;
// 	if (e.key == 'ArrowRight')
// 		dx = 5;

// 	if (dx == 0 && dy == 0)
// 		return;
// 	e.preventDefault();

// 	fetch('game/move', {
// 		method: 'POST',
// 		body: JSON.stringify({ dx, dy })
// 	});
// });

// function readState() {
// 	const gameState = JSON.parse(text);
//
// if (gameState.type === "move")
// {
// 		let penguin = docum(gameState.id);
//			penguin.style.x = g
//			penguin.style.y = g
//			direction/sprite = g
//			direction/sprite = g
// }
//
// if (JOIN) {
//     how to add penguin
// }
// else if (LEFT) {
// 		how to remove penguin
// }


// }

const json_s = '{"username": "wlucas-f", "x": "10", "y": "50"}'

const obj = JSON.parse(json_s);
const user = obj.username;
const jx = obj.x;
const jy = obj.y;
console.log("Name: ", user, "X: ", jx, "Y: ", jy);

let x = parseInt(jx);
let y = parseInt(jy);

const penguin = document.getElementById('penguin');

const range = 50;

function keyPress(event) {
  if (event.key == 'ArrowUp')
    x = Math.max(0, x - range);
  if (event.key == 'ArrowDown')
    x = Math.min(300 - 80, x + range);
  if (event.key == 'ArrowLeft')
    y = Math.max(0, y - range);
  if (event.key == 'ArrowRight')
    y = Math.min(500 - 60, y + range);
  penguin.style.top = `${x}px`;
  penguin.style.left = `${y}px`;
}

document.addEventListener('keyup', keyPress);


function startGame() {
	const canvas = document.getElementById('gameCanvas');

	canvas.style.backgroundImage = "url('images/town.webp')";
	canvas.style.backgroundPosition = "center";
}
