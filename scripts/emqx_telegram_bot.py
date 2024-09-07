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

# Store user state
user_state = {}

async def start(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """Send a message when the command /start is issued."""
    chat_id = update.effective_chat.id
    user = update.effective_user
    message = await update.message.reply_markdown_v2(
        fr'Hi {user.mention_markdown_v2()}\! I am your motor control bot\. Use the buttons below to control the motor\.',
    )
    await show_main_menu(message)

    # Set chat-specific commands
    commands = [
        BotCommand("help", "Show help message"),
        BotCommand("motor_speed", "Set motor speed, usage: /motor_speed <SINGLE_SPEED|INFINITE_SPEED> <speed>")
    ]
    await context.bot.set_my_commands(commands, scope=BotCommandScopeChat(chat_id))

async def show_main_menu(message) -> None:
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

async def show_speed_menu(message, speed_type) -> None:
    """Show a menu for increasing or decreasing speed."""
    keyboard = [
        [InlineKeyboardButton("Increase", callback_data=f'INC_{speed_type}')],
        [InlineKeyboardButton("Decrease", callback_data=f'DEC_{speed_type}')],
        [InlineKeyboardButton("Back", callback_data='BACK_TO_MAIN')]
    ]
    reply_markup = InlineKeyboardMarkup(keyboard)
    await message.reply_text(f'Set {speed_type} Speed:', reply_markup=reply_markup)

async def button(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """Handle button clicks."""
    query = update.callback_query
    await query.answer()

    if query.data in ['START_SINGLE', 'START_INFINITE', 'STOP_INFINITE']:
        payload = json.dumps({"state": query.data})
        mqtt_client.publish(MQTT_TOPIC_STATE, payload)
        await query.edit_message_text(text=f"Motor state set to {query.data}")
        await show_main_menu(query.message)

    elif query.data == 'SET_SINGLE_SPEED':
        context.user_data['speed_type'] = 'SINGLE_SPEED'
        await show_speed_menu(query.message, 'Single')

    elif query.data == 'SET_INFINITE_SPEED':
        context.user_data['speed_type'] = 'INFINITE_SPEED'
        await show_speed_menu(query.message, 'Infinite')

    elif query.data == 'BACK_TO_MAIN':
        await show_main_menu(query.message)
    
    elif query.data.startswith('INC_') or query.data.startswith('DEC_'):
        speed_type_key = context.user_data.get('speed_type')
        speed_change = 10 if 'INC_' in query.data else -10
        # Logic to adjust the speed by increment or decrement
        current_speed = context.user_data.get(speed_type_key, 0)  # Retrieve current speed from context
        new_speed = max(0, current_speed + speed_change)  # Ensure the speed doesn't go below 0
        context.user_data[speed_type_key] = new_speed  # Save new speed

        # Send speed update to MQTT
        payload = json.dumps({"type": speed_type_key, "speed": new_speed})
        mqtt_client.publish(MQTT_TOPIC_SPEED, payload)
        await query.edit_message_text(text=f'{speed_type_key} speed adjusted to {new_speed}')
        await show_speed_menu(query.message, speed_type_key.split('_')[0].capitalize())

async def help_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """Send a message when the command /help is issued."""
    user = update.effective_user
    message = await update.message.reply_markdown_v2(
        fr'Hi {user.mention_markdown_v2()}\! I am your motor control bot\. Use the buttons below to control the motor\.',
    )
    await show_main_menu(message)

async def set_motor_speed(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """Get the motor speed from the user and send it to the MQTT broker."""
    if len(context.args) == 2 and context.args[1].isdigit() and context.args[0] in ['SINGLE_SPEED', 'INFINITE_SPEED']:
        speed_type = context.args[0]
        speed_value = int(context.args[1])
        context.user_data[speed_type] = speed_value  # Save speed in user data
        payload = json.dumps({"type": speed_type, "speed": speed_value})
        mqtt_client.publish(MQTT_TOPIC_SPEED, payload)
        message = await update.message.reply_text(f'Motor speed set to {speed_value} with type {speed_type}')
    else:
        message = await update.message.reply_text('Usage: /motor_speed <SINGLE_SPEED|INFINITE_SPEED> <speed>')
    
    await show_main_menu(message)

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