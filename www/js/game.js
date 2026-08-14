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

function startGame() {

}

const BACKEND = ""

const arena = document.getElementById("arena");

function renderState(state) {
	let ids = [];

	state.forEach(({ username, x, y }) => {
		const id = `penguin-${username}`;
		ids.push(id);
		let el = arena.querySelector('#' + id);
		if (el == null) {
			el = htmlToElement(`<div class="penguin" id="${id}">🐧<p>${username}</p></div>`);
			arena.append(el);
		}
		el.style.top = `${y}px`;
		el.style.left = `${x}px`;
	});

	arena.querySelectorAll('.penguin').forEach(el => ids.includes(el.id) || el.remove());
}

const eu = { username: "meuovo", x: 0, y: 0 }

async function update(eu)
{
	try {
		renderState(await fetch(`${BACKEND}/update`, {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify(eu)
		}).then(data => data.json()))
	} catch { }
}



function htmlToElement(htmlText)
{
	const tmp = document.createElement("div");
	tmp.innerHTML = htmlText;
	const child = tmp.firstElementChild;
	child.remove();
	return child;
}

const range = 40;

function keyPress(event) {
  if (event.key == 'ArrowUp')
    eu.y = Math.max(0, eu.y - range);
  if (event.key == 'ArrowDown')
    eu.y = Math.min(300 - 80, eu.y + range);
  if (event.key == 'ArrowLeft')
    eu.x = Math.max(0, eu.x - range);
  if (event.key == 'ArrowRight')
    eu.x = Math.min(500 - 60, eu.x + range);

	update(eu);
	// penguin.style.left =
	// 	penguin.style.top = ;
}

document.addEventListener('keyup', keyPress);


setInterval(async function() {
	try {
		renderState(await fetch(`${BACKEND}/state`).then(data => data.json()))
	} catch {}
}, 200);
