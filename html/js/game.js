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


function startGame() {
	const canvas = document.getElementById('gameCanvas');
	const ctx = canvas.getContext('2d');

	const image = new Image();
	image.src = 'images/town.webp';

	image.onload = () => {
		ctx.drawImage(image, 0, 0, canvas.width, canvas.height);
	};
}
