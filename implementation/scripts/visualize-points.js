const {Image} = require('imagescript');
const fs = require('fs')
const path = require('path')

const RELATIVE_DATA_DIR = '../data/generated'
const NAME = "clustered"
const OUTPUT_WIDTH = 800
const OUTPUT_HEIGHT = 800

const run = async () => {
    const pointsCsv = fs.readFileSync(path.resolve(`${RELATIVE_DATA_DIR}/${NAME}.points`)).toString()
    const points = pointsCsv.split("\n").map(line => {
      const items = line.split(";")
      return { 
        x: parseFloat(items[0]),
        y: parseFloat(items[1]),
        c: parseInt(items[2]),
      }
    })

    console.log(points[0])

    const colorsCount = Math.max(...points.map(p => p.c)) + 1
    console.log(colorsCount)
    const colors = Array.from({length: colorsCount + 1}).map(_ => {
      return Image.rgbToColor(Math.floor(Math.random() * 256), Math.floor(Math.random() * 256), Math.floor(Math.random() * 256))
    })
    
    for (const c of colors) console.log(c.toString(16))

    const image = new Image(OUTPUT_WIDTH, OUTPUT_HEIGHT);
    image.drawBox(0, 0, OUTPUT_WIDTH, OUTPUT_HEIGHT, 0xFFFFFFFF);

    const MIN_X_VAL = Math.min(...points.map(({x}) => x))
    const MAX_X_VAL = Math.max(...points.map(({x}) => x))
    const MIN_Y_VAL = Math.min(...points.map(({y}) => y))
    const MAX_Y_VAL = Math.max(...points.map(({y}) => y))

    points.forEach( point => 
      image.drawBox(
        Math.round((point.x - MIN_X_VAL) / (MAX_X_VAL - MIN_X_VAL) * OUTPUT_WIDTH),
        OUTPUT_HEIGHT - Math.round((point.y - MIN_Y_VAL) / (MAX_Y_VAL - MIN_Y_VAL) * OUTPUT_HEIGHT),
        5, 5, colors[point.c]
      )
    )
    
    const encoded = await image.encode()

    await fs.writeFileSync(`../figs/renders/${NAME}.png`, encoded);
}

run();