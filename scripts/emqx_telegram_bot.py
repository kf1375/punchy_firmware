import logging
import json
from queue import Queue
from telegram import Update, InlineKeyboardButton, InlineKeyboardMarkup, BotCommand, BotCommandScopeChat
from telegram.ext import Application, CommandHandler, CallbackQueryHandler, ContextTypes
import paho.mqtt.client as mqtt

# Enable logging
logging.basicConfig(
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    level=logging.INFO
)
logger = logging.getLogger(__name__)

# Telegram bot token
BOT_TOKEN = '6599928755:AAEVXPIrKmp3qOvmoT5x5eKXeifYZN01Ce4'

# MQTT configuration
MQTT_BROKER = 'x6e1f6a7.ala.eu-central-1.emqxsl.com'
MQTT_PORT = 8883  # Use the secure port
MQTT_TOPIC_STATE = 'esp32/motorState'
MQTT_TOPIC_SPEED = 'esp32/motorSpeed'
MQTT_USERNAME = 'mqttx_client'
MQTT_PASSWORD = 'public'
CA_CERT = 'data/emqxsl-ca.crt'

# MQTT client setup
mqtt_client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)

# Set CA certificate
mqtt_client.tls_set(ca_certs=CA_CERT)

# Set username and password
mqtt_client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)

async def start(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """Send a message when the command /start is issued."""
    chat_id = update.effective_chat.id
    user = update.effective_user
    message = await update.message.reply_markdown_v2(
        fr'Hi {user.mention_markdown_v2()}\! I am your motor control bot\. Use the buttons below to control the motor\.',
    )
    await show_menu(message)

    # Set chat-specific commands
    commands = [
        BotCommand("help", "Show help message"),
        BotCommand("motor_speed", "Set motor speed, usage: /motor_speed <SINGLE_SPEED|INFINITE_SPEED> <speed>")
    ]
    await context.bot.set_my_commands(commands, scope=BotCommandScopeChat(chat_id))

async def show_menu(message) -> None:
    """Show a menu of commands using inline buttons."""
    keyboard = [
        [InlineKeyboardButton("Start Single", callback_data='START_SINGLE')],
        [InlineKeyboardButton("Start Infinite", callback_data='START_INFINITE')],
        [InlineKeyboardButton("Stop Infinite", callback_data='STOP_INFINITE')],
        [InlineKeyboardButton("Set Single Speed", callback_data='SET_SINGLE_SPEED')],
        [InlineKeyboardButton("Set Infinite Speed", callback_data='SET_INFINITE_SPEED')]
    ]
    reply_markup = InlineKeyboardMarkup(keyboard)
    await message.reply_text('Please choose:', reply_markup=reply_markup)

async def button(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """Handle button clicks."""
    query = update.callback_query
    await query.answer()

    if query.data in ['START_SINGLE', 'START_INFINITE', 'STOP_INFINITE']:
        payload = json.dumps({"state": query.data})
        mqtt_client.publish(MQTT_TOPIC_STATE, payload)
        await query.edit_message_text(text=f"Motor state set to {query.data}")

    elif query.data == 'SET_SINGLE_SPEED':
        await query.edit_message_text(text="Please send the speed using /motor_speed SINGLE_SPEED <speed>")
    
    elif query.data == 'SET_INFINITE_SPEED':
        await query.edit_message_text(text="Please send the speed using /motor_speed INFINITE_SPEED <speed>")
    
    await show_menu(query.message)

async def help_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """Send a message when the command /help is issued."""
    user = update.effective_user
    message = await update.message.reply_markdown_v2(
        fr'Hi {user.mention_markdown_v2()}\! I am your motor control bot\. Use the buttons below to control the motor\.',
    )
    await show_menu(message)

async def set_motor_speed(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """Get the motor speed from the user and send it to the MQTT broker."""
    if len(context.args) == 2 and context.args[1].isdigit() and context.args[0] in ['SINGLE_SPEED', 'INFINITE_SPEED']:
        speed_type = context.args[0]
        speed_value = int(context.args[1])
        payload = json.dumps({"type": speed_type, "speed": speed_value})
        mqtt_client.publish(MQTT_TOPIC_SPEED, payload)
        message = await update.message.reply_text(f'Motor speed set to {speed_value} with type {speed_type}')
    else:
        message = await update.message.reply_text('Usage: /motor_speed <SINGLE_SPEED|INFINITE_SPEED> <speed>')
    
    await show_menu(message)

def main() -> None:
    """Start the bot."""
    application = Application.builder().token(BOT_TOKEN).build()

    # Connect to the MQTT broker
    mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
    mqtt_client.loop_start()

    # Register handlers
    application.add_handler(CommandHandler("start", start))
    application.add_handler(CommandHandler("help", help_command))
    application.add_handler(CommandHandler("motor_speed", set_motor_speed))
    application.add_handler(CallbackQueryHandler(button))

    # Start the Bot
    application.run_polling(allowed_updates=Update.ALL_TYPES)

if __name__ == '__main__':
    main()