const { Image } = require('imagescript');
const fs = require('fs')
const path = require('path')

const RELATIVE_DATA_DIR = '../data/osm/raw'
const RELATIVE_OUTPUT_DIR = '../data/osm'
const FILENAMES = ['1', '2', '3', '4', '5', '6', '7', '8', '9', '10']
const FILE_EXTENSION = '.png'

const SAMPLING_RATE = 3 // sample every xth pixel horizontally / vertically
const MAX_OFFSET = 0.05 // Offset used to slightly distort pixel positions preventing duplicate x/y coordinates

// Map of OSM colors to our colors
// Anything not defined in this map will be treated as a null pixel and not be included
const COLORS = [
  "#B5D0D0", // water
  "#CDEAB0", // grass
  "#ACD09D", // forest
  "#E892A2", // highway
  "#FFFFFF", // road-ish
  "#F6F9BE", // arterial road
  "#F1D9D8", // industry
  "#F6EEB6", // industry: building 1
  "#D9D0C9", // residential / industry
  "#89D2AE", // misc building
  "#33CC99", // sport
  "#F6F9BE", // parking lot?
]

function rgbaToRGB(color) {
  return '#'+ (color >> 8 & 0xFFFFFF).toString(16).toUpperCase();
}

function getRandomSample(array, n) {
  let shuffled = array.slice();

  for (let i = shuffled.length - 1; i > 0; i--) {
    const j = Math.floor(Math.random() * (i + 1));
    [shuffled[i], shuffled[j]] = [shuffled[j], shuffled[i]];
  }

  return shuffled.slice(0, n);
}

const run = async(name) => {
  const buff = fs.readFileSync(path.resolve(`${RELATIVE_DATA_DIR}/${name}${FILE_EXTENSION}`))
  const input = await Image.decode(buff)

  const points = []
  
  for (let y = 1; y < input.height; y += SAMPLING_RATE) {
    for (let x = 1; x < input.width; x += SAMPLING_RATE) {
      // Get pixel value at coordinate
      const val = rgbaToRGB(input.getPixelAt(x, y));
      const c = COLORS.indexOf(val)
      if (c != -1) {
        const xOffset = -MAX_OFFSET + Math.random() * 2 * MAX_OFFSET 
        const yOffset = -MAX_OFFSET + Math.random() * 2 * MAX_OFFSET 
        points.push({ x: x + xOffset, y: input.height - y + yOffset, c })
      }
    }
  }

  // output points
  let lines = points.map(({ x, y, c }) => `${x};${y};${c}`)
  lines = getRandomSample(lines, 50000);
  fs.writeFileSync(path.resolve(`${RELATIVE_OUTPUT_DIR}/${name}.points`), lines.filter(x => x.length).join("\n"))
}

FILENAMES.forEach(async name => await run(name))