const { runPandoc } = require('pandoc-installer-wrapper');

runPandoc(['--version'])
  .then(output => console.log('Pandoc version:', output))
  .catch(err => console.error('Error running Pandoc:', err));