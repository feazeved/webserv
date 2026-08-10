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


let x = 0;
let y = 0;

const penguin = document.getElementById('penguin');


function keyPress(event) {
	if (event.key == 'ArrowUp')
		y -= 5;
	if (event.key == 'ArrowDown')
		y += 5;
	if (event.key == 'ArrowLeft')
		x -= 5;
	if (event.key == 'ArrowRight')
		x += 5;
	penguin.style.top = `${x}px`;
	penguin.style.left = `${y}px`;
}

document.addEventListener('keydown', keyPress);


function startGame() {
	const canvas = document.getElementById('gameCanvas');

	canvas.style.backgroundImage = "url('images/town.webp')";
	canvas.style.backgroundPosition = "center";
}
