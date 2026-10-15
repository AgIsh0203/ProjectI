/**
 * Squirrel Wheels playtest questionnaire as a Google Form.
 *
 * Run once from https://script.google.com (New project, paste this, Run `makeForm`, approve the
 * Forms/Sheets permissions). It creates the form plus a response spreadsheet in your Drive and
 * logs both links (View > Logs, or the Execution log). Re-running makes a second form.
 */
function makeForm() {
  const form = FormApp.create('Squirrel Wheels: how was the drive?')
    .setDescription(
      'Thanks for playing! This takes about three minutes. Answer straight after your run, ' +
      'while the chaos is fresh. Honest beats nice: the annoying bits are the most useful.')
    .setCollectEmail(false)
    .setLimitOneResponsePerUser(false)
    .setProgressBar(true)
    .setConfirmationMessage('Thanks! That helps a lot. Go grab another run.');

  form.addTextItem().setTitle('Your name (or Discord handle)').setRequired(true);

  form.addMultipleChoiceItem().setTitle('Would you play another run with this group?')
    .setChoiceValues(['Yes, right now', 'Maybe another day', 'No']).setRequired(true);

  form.addScaleItem().setTitle('How fun was it overall?')
    .setBounds(1, 5).setLabels('a chore', "couldn't stop laughing").setRequired(true);

  form.addParagraphTextItem().setTitle('What was the funniest moment?')
    .setHelpText('Who got thrown out, where, and why it was funny.');

  form.addParagraphTextItem().setTitle('What was the most annoying thing?')
    .setHelpText('Annoying-funny or just annoying? Both are useful.');

  const repairs = ['Engine (mash)', 'Brakes (timing ring)', 'Tire (hold and get dragged)', 'Door (slam)', 'Flip-up (push together)'];

  form.addPageBreakItem().setTitle('The jobs');

  form.addCheckboxItem().setTitle('Which jobs did you do?').setHelpText('Tick everything you tried.')
    .setChoiceValues(['Wheel (steering)', 'Pedals (gas and brake)', 'Fixing parts', 'Hanging on to a tire',
      'Pushing the car upright', 'Mostly falling out']);

  form.addMultipleChoiceItem().setTitle('Best repair job?').setChoiceValues(repairs);
  form.addMultipleChoiceItem().setTitle('Worst repair job?').setChoiceValues(repairs);

  form.addMultipleChoiceItem().setTitle('In the first minute, did you know what to do?')
    .setChoiceValues(['Yes', 'Took a bit', 'No, I was lost']);
  form.addTextItem().setTitle('What confused you? (optional)');

  form.addPageBreakItem().setTitle('The game');

  form.addMultipleChoiceItem().setTitle('Acorn in Mouth (muted for 20 seconds): funny or annoying?')
    .setChoiceValues(['Funny', 'Annoying', "Didn't notice it"]);

  form.addMultipleChoiceItem().setTitle("How did the run's length feel?")
    .setChoiceValues(['Too short', 'About right', 'Dragged on']);

  form.addMultipleChoiceItem().setTitle('Would you watch a streamer play this?')
    .setChoiceValues(['Yes', 'Maybe', 'No']);

  form.addParagraphTextItem().setTitle('Anything broken?')
    .setHelpText('Crashes, lag, voice cutting out, getting stuck, a button that did nothing. What happened, and roughly when.');
  form.addTextItem().setTitle('Your graphics card, if you know it (optional)');

  form.addTextItem().setTitle('Got a better name for the game?')
    .setHelpText('"Squirrel Wheels" is a working title. Squirrel or wheels puns welcome.');

  const sheet = SpreadsheetApp.create('Squirrel Wheels playtest answers');
  form.setDestination(FormApp.DestinationType.SPREADSHEET, sheet.getId());

  Logger.log('Share this link with friends: ' + form.getPublishedUrl());
  Logger.log('Edit the form: ' + form.getEditUrl());
  Logger.log('Answers spreadsheet: ' + sheet.getUrl());
}
